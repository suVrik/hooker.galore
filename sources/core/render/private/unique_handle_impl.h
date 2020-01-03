#pragma once

#include "core/render/unique_handle.h"

namespace hg {

template <typename T>
UniqueHandle<T>::UniqueHandle()
    : m_handle(BGFX_INVALID_HANDLE) {
}

template <typename T>
UniqueHandle<T>::UniqueHandle(T handle)
    : m_handle(handle) {
}

template <typename T>
UniqueHandle<T>::UniqueHandle(UniqueHandle&& original)
    : m_handle(original.m_handle) {
    original.m_handle = BGFX_INVALID_HANDLE;
}

template <typename T>
UniqueHandle<T>::~UniqueHandle() {
    if (bgfx::isValid(m_handle)) {
        bgfx::destroy(m_handle);
    }
}

template <typename T>
UniqueHandle<T>& UniqueHandle<T>::operator=(UniqueHandle&& original) {
    if (&original != this) {
        if (bgfx::isValid(m_handle)) {
            bgfx::destroy(m_handle);
        }
        m_handle = original.m_handle;
        original.m_handle = BGFX_INVALID_HANDLE;
    }
    return *this;
}

template <typename T>
T UniqueHandle<T>::operator*() const {
    return m_handle;
}

} // namespace hg
