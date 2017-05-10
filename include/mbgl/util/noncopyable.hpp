#pragma once

namespace mbgl { namespace util {

namespace non_copyable_
{

class noncopyable
{
protected:
    constexpr noncopyable() = default;
    ~noncopyable() = default;
    noncopyable( noncopyable const& ) = delete;
    noncopyable& operator=(noncopyable const& ) = delete;
};
} // namespace non_copyable_

using noncopyable = non_copyable_::noncopyable;

} // namespace util
} // namespace mbgl
