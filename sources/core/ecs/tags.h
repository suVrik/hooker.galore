#pragma once

#include <string>
#include <vector>

namespace hg {

template <typename U, typename V>
class TagAnd;
template <typename U, typename V>
class TagOr;
template <typename U>
class TagNot;

/** Each World has a set of tags. Each ECS system defines its own "tag expression": an expression that checks whether
    this system should be executed in a World based on its tags. A tag, in turn, is just a unique identifier.
    
    Tag render("render");
    Tag imgui("imgui");
    Tag release("release");
    auto visual_debug_system = render && imgui && !release;
    auto another_visual_system = render && (imgui || release); */
class Tag final {
public:
    /** Return the total number of tags registered. */
    static size_t get_tags_count();

    /** Return tag registered under specified index. */
    static Tag get_tag_by_index(size_t index);

    /** Create a unique tag. Do not create unnecessary tags to avoid huge tag lists.
        If tag is inheritable, it automatically goes from parent World to child World.
        If tag is propagable, it automatically goes from child World to parent World. */
    explicit Tag(const std::string& name, bool is_inheritable = false, bool is_propagable = false);

    /** Return unique tag index. */
    size_t get_index() const;

    /** Return name of this tag. */
    const std::string& get_name() const;

    /** Check whether this tag is inheritable, which means it automatically goes from parent World to child World. */
    bool is_inheritable() const;

    /** Check whether this tag is propagable, which means it automatically goes from child World to parent World. */
    bool is_propagable() const;

    /** Construct a tag expression that matches a tag list only if this tag is present in the tag list and specified
        expression matches the tag list too. */
    template <typename T>
    TagAnd<Tag, T> operator&&(const T& t) const;
    
    /** Construct a tag expression that matches a tag list if this tag is present in the tag list or specified
        expression matches the tag list. */
    template <typename T>
    TagOr<Tag, T> operator||(const T& t) const;
    
    /** Construct a tag expression that matches a tag list if this tag is not present in it. */
    TagNot<Tag> operator!() const;

    /** Check whether this tag is present in the specified tag list. */
    bool test(const std::vector<bool>& tags) const;

private:
    struct TagDescriptor final {
        std::string name;
        bool is_inheritable;
        bool is_propagable;
    };

    static std::vector<TagDescriptor> descriptors;

    Tag(size_t index);

    size_t m_index;
};

/** `TagAnd` is a tag expression that works like logical AND on two other tag expressions. */
template <typename U, typename V>
class TagAnd final {
public:
    /** Construct a tag expression that matches a tag list only if both of specified expressions match the tag list. */
    explicit TagAnd(const U& u, const V& v);
    
    /** Construct a tag expression that matches a tag list only if this tag expression matches the tag list and
        specified expression matches the tag list too. */
    template <typename T>
    TagAnd<TagAnd<U, V>, T> operator&&(const T& t) const;
    
    /** Construct a tag expression that matches a tag list if this tag expression matches the tag list or specified
        expression matches the tag list. */
    template <typename T>
    TagOr<TagAnd<U, V>, T> operator||(const T& t) const;

    /** Construct a tag expression that matches a tag list if this tag expression doesn't match it. */
    TagNot<TagAnd<U, V>> operator!() const;

    /** Check whether this tag expression matches the specified tag list. */
    bool test(const std::vector<bool>& tags) const;

private:
    U m_u;
    V m_v;
};

/** `TagOr` is a tag expression that works like logical OR on two other tag expressions. */
template <typename U, typename V>
class TagOr final {
public:
    /** Construct a tag expression that matches a tag list if any of specified expressions matches the tag list. */
    explicit TagOr(const U& u, const V& v);
    
    /** Construct a tag expression that matches a tag list only if this tag expression matches the tag list and
        specified expression matches the tag list too. */
    template <typename T>
    TagAnd<TagOr<U, V>, T> operator&&(const T& t) const;
    
    /** Construct a tag expression that matches a tag list if this tag expression matches the tag list or specified
        expression matches the tag list. */
    template <typename T>
    TagOr<TagOr<U, V>, T> operator||(const T& t) const;

    /** Construct a tag expression that matches a tag list if this tag expression doesn't match it. */
    TagNot<TagOr<U, V>> operator!() const;

    /** Check whether this tag expression matches the specified tag list. */
    bool test(const std::vector<bool>& tags) const;

private:
    U m_u;
    V m_v;
};

/** `TagNot` is a tag expression that inverts the result of another tag expression. */
template <typename U>
class TagNot final {
public:
    /** Construct a tag expression that matches a tag list if specified tag expression doesn't match it. */
    explicit TagNot(const U& u);
    
    /** Construct a tag expression that matches a tag list only if this tag expression matches the tag list and
        specified expression matches the tag list too. */
    template <typename T>
    TagAnd<TagNot<U>, T> operator&&(const T& t) const;
    
    /** Construct a tag expression that matches a tag list if this tag expression matches the tag list or specified
        expression matches the tag list. */
    template <typename T>
    TagOr<TagNot<U>, T> operator||(const T& t) const;

    /** Construct a tag expression that matches a tag list if this tag expression doesn't match it. */
    U operator!() const;

    /** Check whether this tag expression matches the specified tag list. */
    bool test(const std::vector<bool>& tags) const;

private:
    U m_u;
};

/** `TagWrapper` is a template-free abstraction of any tag expression.

    Tag render("render");
    Tag imgui("imgui");
    Tag release("release");
    auto wrapper = std::unique_ptr<TagWrapper>(new TagWrapperTemplate(render && imgui && !release)); 
    const bool result = wrapper->test(std::vector<bool>{ render, imgui, release }); */
class TagWrapper {
public:
    /** Check whether the underlying tag expression matches the specified tag list. */
    virtual bool test(const std::vector<bool>& tags) const = 0;
};

/** `TagWrapperTemplate` is a virtual inheritor of `TagWrapper` - a template-free abstraction of any tag expression. */
template <typename T>
class TagWrapperTemplate final : public TagWrapper {
public:
    /** Construct `TagWrapperTemplate` from specified tag expression. */
    explicit TagWrapperTemplate(const T& t);

    /** Check whether the underlying tag expression matches the specified tag list. */
    bool test(const std::vector<bool>& tags) const final;

private:
    T m_t;
};

} // namespace hg

#include "core/ecs/private/tags_impl.h"
