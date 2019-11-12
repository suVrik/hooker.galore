#pragma once

#include "core/ecs/tags.h"

namespace hg {

template <typename T>
TagAnd<Tag, T> Tag::operator&&(const T& t) const {
    return TagAnd<Tag, T>(*this, t);
}

template <typename T>
TagOr<Tag, T> Tag::operator||(const T& t) const {
    return TagOr<Tag, T>(*this, t);
}

inline TagNot<Tag> Tag::operator!() const {
    return TagNot<Tag>(*this);
}

inline bool Tag::test(const std::vector<bool>& tags) const {
    return tags.size() > m_index && tags[m_index];
}

//////////////////////////////////////////////////////////////////////////

template <typename U, typename V>
TagAnd<U, V>::TagAnd(const U& u, const V& v)
        : m_u(u)
        , m_v(v) {
}

template <typename U, typename V>
template <typename T>
TagAnd<TagAnd<U, V>, T> TagAnd<U, V>::operator&&(const T& t) const {
    return TagAnd<TagAnd<U, V>, T>(*this, t);
}

template <typename U, typename V>
template <typename T>
TagOr<TagAnd<U, V>, T> TagAnd<U, V>::operator||(const T& t) const {
    return TagOr<TagAnd<U, V>, T>(*this, t);
}

template <typename U, typename V>
TagNot<TagAnd<U, V>> TagAnd<U, V>::operator!() const {
    return TagNot<TagAnd<U, V>>(*this);
}

template <typename U, typename V>
bool TagAnd<U, V>::test(const std::vector<bool>& tags) const {
    return m_u.test(tags) && m_v.test(tags);
}

//////////////////////////////////////////////////////////////////////////

template <typename U, typename V>
TagOr<U, V>::TagOr(const U& u, const V& v)
        : m_u(u)
        , m_v(v) {
}

template <typename U, typename V>
template <typename T>
TagAnd<TagOr<U, V>, T> TagOr<U, V>::operator&&(const T& t) const {
    return TagAnd<TagOr<U, V>, T>(*this, t);
}

template <typename U, typename V>
template <typename T>
TagOr<TagOr<U, V>, T> TagOr<U, V>::operator||(const T& t) const {
    return TagOr<TagOr<U, V>, T>(*this, t);
}

template <typename U, typename V>
TagNot<TagOr<U, V>> TagOr<U, V>::operator!() const {
    return TagNot<TagOr<U, V>>(*this);
}

template <typename U, typename V>
bool TagOr<U, V>::test(const std::vector<bool>& tags) const {
    return m_u.test(tags) || m_v.test(tags);
}

//////////////////////////////////////////////////////////////////////////

template <typename U>
TagNot<U>::TagNot(const U& u) 
        : m_u(u) {
}

template <typename U>
template <typename T>
TagAnd<TagNot<U>, T> TagNot<U>::operator&&(const T& t) const {
    return TagAnd<TagNot<U>, T>(*this, t);
}

template <typename U>
template <typename T>
TagOr<TagNot<U>, T> TagNot<U>::operator||(const T& t) const {
    return TagOr<TagNot<U>, T>(*this, t);
}

template <typename U>
U TagNot<U>::operator!() const {
    return m_u;
}

template <typename U>
bool TagNot<U>::test(const std::vector<bool>& tags) const {
    return !m_u.test(tags);
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
TagWrapperTemplate<T>::TagWrapperTemplate(const T& t) 
    : m_t(t) {
}

template <typename T>
bool TagWrapperTemplate<T>::test(const std::vector<bool>& tags) const {
    return m_t.test(tags);
}

} // namespace hg
