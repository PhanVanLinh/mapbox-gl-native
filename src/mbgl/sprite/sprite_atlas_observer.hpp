#pragma once

#include <exception>
#include <unordered_map>
#include <memory>

namespace mbgl {

namespace style {
class Image;
} // namespace style

class SpriteAtlasObserver {
public:
    using Images = std::unordered_map<std::string, std::shared_ptr<const style::Image>>;

    virtual ~SpriteAtlasObserver() = default;

    virtual void onSpriteLoaded(const Images&) {}
    virtual void onSpriteError(std::exception_ptr) {}
};

} // namespace mbgl
