#pragma once

#include <mbgl/util/noncopyable.hpp>
#include <mbgl/style/image.hpp>

#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <array>
#include <memory>

namespace mbgl {

class Scheduler;
class FileSource;
class SpriteAtlasObserver;

class SpriteAtlas : public util::noncopyable {
public:
    using Images = std::unordered_map<std::string, std::unique_ptr<style::Image>>;

    SpriteAtlas(float pixelRatio);
    ~SpriteAtlas();

    void load(const std::string& url, Scheduler&, FileSource&);

    void markAsLoaded() {
        loaded = true;
    }

    bool isLoaded() const {
        return loaded;
    }

    void dumpDebugLogs() const;

    void setObserver(SpriteAtlasObserver*);

    const style::Image* getImage(const std::string&) const;
    // Returns the image if actually added
    std::shared_ptr<const style::Image> addImage(const std::string&, std::unique_ptr<style::Image>);
    // Returns true if actually removed
    bool removeImage(const std::string&);

    float getPixelRatio() const { return pixelRatio; }

private:
    void emitSpriteLoadedIfComplete();

    // Invoked by SpriteAtlasWorker
    friend class SpriteAtlasWorker;
    void onParsed(Images&& result);
    void onError(std::exception_ptr);

    const float pixelRatio;

    struct Loader;
    std::unique_ptr<Loader> loader;

    bool loaded = false;

    SpriteAtlasObserver* observer = nullptr;

    std::unordered_map<std::string, std::shared_ptr<const style::Image>> entries;
};

} // namespace mbgl
