#include <mbgl/test/util.hpp>
#include <mbgl/test/fixture_log_observer.hpp>
#include <mbgl/test/stub_file_source.hpp>
#include <mbgl/test/stub_style_observer.hpp>

#include <mbgl/sprite/sprite_atlas.hpp>
#include <mbgl/sprite/sprite_atlas.hpp>
#include <mbgl/sprite/sprite_parser.hpp>
#include <mbgl/util/io.hpp>
#include <mbgl/util/image.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/default_thread_pool.hpp>
#include <mbgl/util/string.hpp>

#include <utility>

using namespace mbgl;

TEST(SpriteAtlas, OtherPixelRatio) {
    FixtureLog log;
    SpriteAtlas atlas(1);

    // Adding mismatched sprite image
    atlas.addImage("one", std::make_unique<style::Image>(PremultipliedImage({ 8, 8 }), 2));
}

TEST(SpriteAtlas, Replace) {
    FixtureLog log;
    SpriteAtlas atlas(1);

    atlas.addImage("sprite", std::make_unique<style::Image>(PremultipliedImage({ 16, 16 }), 2));
    auto image = atlas.getImage("sprite");
    atlas.addImage("sprite", std::make_unique<style::Image>(PremultipliedImage({ 16, 16 }), 2));
    EXPECT_NE(image, atlas.getImage("sprite"));
}

TEST(SpriteAtlas, ReplaceWithDifferentDimensions) {
    FixtureLog log;
    SpriteAtlas atlas(1);

    atlas.addImage("sprite", std::make_unique<style::Image>(PremultipliedImage({ 16, 16 }), 2));
    atlas.addImage("sprite", std::make_unique<style::Image>(PremultipliedImage({ 18, 18 }), 2));

    EXPECT_EQ(1u, log.count({
                                    EventSeverity::Warning,
                                    Event::Sprite,
                                    int64_t(-1),
                                    "Can't change sprite dimensions for 'sprite'",
                            }));
}

class SpriteAtlasTest {
public:
    SpriteAtlasTest() = default;

    util::RunLoop loop;
    StubFileSource fileSource;
    StubStyleObserver observer;
    ThreadPool threadPool { 1 };
    SpriteAtlas spriteAtlas{ 1 };

    void run() {
        // Squelch logging.
        Log::setObserver(std::make_unique<Log::NullObserver>());

        spriteAtlas.setObserver(&observer);
        spriteAtlas.load("test/fixtures/resources/sprite", threadPool, fileSource);

        loop.run();
    }

    void end() {
        loop.stop();
    }
};

Response successfulSpriteImageResponse(const Resource& resource) {
    EXPECT_EQ("test/fixtures/resources/sprite.png", resource.url);
    Response response;
    response.data = std::make_unique<std::string>(util::read_file(resource.url));
    return response;
}

Response successfulSpriteJSONResponse(const Resource& resource) {
    EXPECT_EQ("test/fixtures/resources/sprite.json", resource.url);
    Response response;
    response.data = std::make_unique<std::string>(util::read_file(resource.url));
    return response;
}

Response failedSpriteResponse(const Resource&) {
    Response response;
    response.error = std::make_unique<Response::Error>(
        Response::Error::Reason::Other,
        "Failed by the test case");
    return response;
}

Response corruptSpriteResponse(const Resource&) {
    Response response;
    response.data = std::make_unique<std::string>("CORRUPT");
    return response;
}

TEST(SpriteAtlas, LoadingSuccess) {
    SpriteAtlasTest test;

    test.fileSource.spriteImageResponse = successfulSpriteImageResponse;
    test.fileSource.spriteJSONResponse = successfulSpriteJSONResponse;

    test.observer.spriteError = [&] (std::exception_ptr error) {
        FAIL() << util::toString(error);
        test.end();
    };

    test.observer.spriteLoaded = [&] (const SpriteAtlasObserver::Images&) {
        EXPECT_EQ(1.0, test.spriteAtlas.getPixelRatio());
        EXPECT_TRUE(test.spriteAtlas.isLoaded());
        test.end();
    };

    test.run();
}

TEST(SpriteAtlas, JSONLoadingFail) {
    SpriteAtlasTest test;

    test.fileSource.spriteImageResponse = successfulSpriteImageResponse;
    test.fileSource.spriteJSONResponse = failedSpriteResponse;

    test.observer.spriteError = [&] (std::exception_ptr error) {
        EXPECT_TRUE(error != nullptr);
        EXPECT_EQ("Failed by the test case", util::toString(error));
        EXPECT_FALSE(test.spriteAtlas.isLoaded());
        test.end();
    };

    test.run();
}

TEST(SpriteAtlas, ImageLoadingFail) {
    SpriteAtlasTest test;

    test.fileSource.spriteImageResponse = failedSpriteResponse;
    test.fileSource.spriteJSONResponse = successfulSpriteJSONResponse;

    test.observer.spriteError = [&] (std::exception_ptr error) {
        EXPECT_TRUE(error != nullptr);
        EXPECT_EQ("Failed by the test case", util::toString(error));
        EXPECT_FALSE(test.spriteAtlas.isLoaded());
        test.end();
    };

    test.run();
}

TEST(SpriteAtlas, JSONLoadingCorrupted) {
    SpriteAtlasTest test;

    test.fileSource.spriteImageResponse = successfulSpriteImageResponse;
    test.fileSource.spriteJSONResponse = corruptSpriteResponse;

    test.observer.spriteError = [&] (std::exception_ptr error) {
        EXPECT_TRUE(error != nullptr);
        EXPECT_EQ("Failed to parse JSON: Invalid value. at offset 0", util::toString(error));
        EXPECT_FALSE(test.spriteAtlas.isLoaded());
        test.end();
    };

    test.run();
}

TEST(SpriteAtlas, ImageLoadingCorrupted) {
    SpriteAtlasTest test;

    test.fileSource.spriteImageResponse = corruptSpriteResponse;
    test.fileSource.spriteJSONResponse = successfulSpriteJSONResponse;

    test.observer.spriteError = [&] (std::exception_ptr error) {
        EXPECT_TRUE(error != nullptr);
        // Not asserting on platform-specific error text.
        EXPECT_FALSE(test.spriteAtlas.isLoaded());
        test.end();
    };

    test.run();
}

TEST(SpriteAtlas, LoadingCancel) {
    SpriteAtlasTest test;

    test.fileSource.spriteImageResponse =
    test.fileSource.spriteJSONResponse = [&] (const Resource&) {
        test.end();
        return optional<Response>();
    };

    test.observer.spriteLoaded = [&] (const SpriteAtlasObserver::Images&) {
        FAIL() << "Should never be called";
    };

    test.run();
}
