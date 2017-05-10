#include <mbgl/sprite/sprite_atlas.hpp>
#include <mbgl/sprite/sprite_atlas_worker.hpp>
#include <mbgl/sprite/sprite_atlas_observer.hpp>
#include <mbgl/sprite/sprite_parser.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/platform.hpp>
#include <mbgl/util/std.hpp>
#include <mbgl/util/constants.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/storage/file_source.hpp>
#include <mbgl/storage/resource.hpp>
#include <mbgl/storage/response.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/actor/actor.hpp>

#include <cassert>

namespace mbgl {

static SpriteAtlasObserver nullObserver;

struct SpriteAtlas::Loader {
    Loader(Scheduler& scheduler, SpriteAtlas& spriteAtlas)
        : mailbox(std::make_shared<Mailbox>(*util::RunLoop::Get())),
          worker(scheduler, ActorRef<SpriteAtlas>(spriteAtlas, mailbox)) {
    }

    std::shared_ptr<const std::string> image;
    std::shared_ptr<const std::string> json;
    std::unique_ptr<AsyncRequest> jsonRequest;
    std::unique_ptr<AsyncRequest> spriteRequest;
    std::shared_ptr<Mailbox> mailbox;
    Actor<SpriteAtlasWorker> worker;
};

SpriteAtlas::SpriteAtlas(float pixelRatio_)
        : pixelRatio(pixelRatio_)
        , observer(&nullObserver) {
}

SpriteAtlas::~SpriteAtlas() = default;

void SpriteAtlas::load(const std::string& url, Scheduler& scheduler, FileSource& fileSource) {
    if (url.empty()) {
        // Treat a non-existent sprite as a successfully loaded empty sprite.
        markAsLoaded();
        SpriteAtlasObserver::Images result;
        observer->onSpriteLoaded(result);
        return;
    }

    loader = std::make_unique<Loader>(scheduler, *this);

    loader->jsonRequest = fileSource.request(Resource::spriteJSON(url, pixelRatio), [this](Response res) {
        if (res.error) {
            observer->onSpriteError(std::make_exception_ptr(std::runtime_error(res.error->message)));
        } else if (res.notModified) {
            return;
        } else if (res.noContent) {
            loader->json = std::make_shared<const std::string>();
            emitSpriteLoadedIfComplete();
        } else {
            // Only trigger a sprite loaded event we got new data.
            loader->json = res.data;
            emitSpriteLoadedIfComplete();
        }
    });

    loader->spriteRequest = fileSource.request(Resource::spriteImage(url, pixelRatio), [this](Response res) {
        if (res.error) {
            observer->onSpriteError(std::make_exception_ptr(std::runtime_error(res.error->message)));
        } else if (res.notModified) {
            return;
        } else if (res.noContent) {
            loader->image = std::make_shared<const std::string>();
            emitSpriteLoadedIfComplete();
        } else {
            loader->image = res.data;
            emitSpriteLoadedIfComplete();
        }
    });
}

void SpriteAtlas::emitSpriteLoadedIfComplete() {
    assert(loader);

    if (!loader->image || !loader->json) {
        return;
    }

    loader->worker.invoke(&SpriteAtlasWorker::parse, loader->image, loader->json);
    // TODO: delete the loader?
}

void SpriteAtlas::onParsed(Images&& parsed) {
    markAsLoaded();

    SpriteAtlasObserver::Images result;
    result.reserve(parsed.size());

    for (auto& pair : parsed) {
        std::shared_ptr<const style::Image> added = addImage(pair.first, std::move(pair.second));
        if (added) {
            result.emplace(pair.first, std::move(added));
        }
    }

    observer->onSpriteLoaded(std::move(result));
}

void SpriteAtlas::onError(std::exception_ptr err) {
    observer->onSpriteError(err);
}

void SpriteAtlas::setObserver(SpriteAtlasObserver* observer_) {
    observer = observer_;
}

void SpriteAtlas::dumpDebugLogs() const {
    Log::Info(Event::General, "SpriteAtlas::loaded: %d", loaded);
}

std::shared_ptr<const style::Image> SpriteAtlas::addImage(const std::string& id, std::unique_ptr<style::Image> image_) {

    auto it = entries.find(id);
    if (it == entries.end()) {
        // Add new
        std::shared_ptr<const style::Image> tmp = std::move(image_);
        it = entries.emplace(id, std::move(tmp)).first;
        return it->second;
    }

    std::shared_ptr<const style::Image>& image = it->second;

    // There is already a sprite with that name in our store.
    if (image->getImage().size != image_->getImage().size) {
        Log::Warning(Event::Sprite, "Can't change sprite dimensions for '%s'", id.c_str());
        return {};
    }

    // Update existing
    image = std::move(image_);

    return image;
}

bool SpriteAtlas::removeImage(const std::string& id) {

    auto it = entries.find(id);
    if (it == entries.end()) {
        return false;
    }

    entries.erase(it);
    return true;
}

const style::Image* SpriteAtlas::getImage(const std::string& id) const {
    const auto it = entries.find(id);
    if (it != entries.end()) {
        return it->second.get();
    }
    if (!entries.empty()) {
        Log::Info(Event::Sprite, "Can't find sprite named '%s'", id.c_str());
    }
    return nullptr;
}

} // namespace mbgl
