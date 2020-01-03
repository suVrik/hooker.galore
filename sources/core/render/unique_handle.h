#pragma once

#include <bgfx/bgfx.h>

namespace hg {

/** `UniqueHandle` is a basic RAII wrapper around bgfx handles. It works exactly like unique_ptr, but with handles. */
template <typename T>
class UniqueHandle {
public:
    UniqueHandle();
    UniqueHandle(T handle);
    UniqueHandle(const UniqueHandle& original) = delete;
    UniqueHandle(UniqueHandle&& original);
    ~UniqueHandle();
    UniqueHandle& operator=(const UniqueHandle& original) = delete;
    UniqueHandle& operator=(UniqueHandle&& original);
    T operator*() const;

private:
    T m_handle;
};

} // namespace hg

#include "core/render/private/unique_handle_impl.h"
