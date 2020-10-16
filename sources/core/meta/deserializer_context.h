#pragma once

#include "core/ecs/world.h"

#include <entt/meta/meta.hpp>
#include <string>

namespace hg {

class Object {
public:
    class Iterator {
    public:
        /** For dictionary return name of specific property.
            For list return string representation of index of specific element. */
        const std::string& get_key() const;

        /** Return value of dictionary property or list element. */
        Object& get_value();
        const Object& get_value() const;
    };
    
    class ConstIterator {
    public:
        /** For dictionary return name of specific property.
            For list return string representation of index of specific element. */
        const std::string& get_key() const;

        /** Return value of dictionary property or list element. */
        const Object& get_value() const;
    };

    /** Return property with the specified `name`. If such property doesn't exist or this is not a dictionary, return
        invalid dictionary. */
    Object& operator[](const std::string& name);
    const Object& operator[](const std::string& name) const;
    
    /** For dictionary return the number of properties in it. For list return the number of elements in it.
        For value return 1. For invalid object return 0. */
    size_t get_size() const;

    /** Check whether this value can be casted to specified type `T`. Supported all integer and floating-point types
        and std::string. Other types will trigger a linkage error. */
    template <typename T>
    bool can_cast() const;

    /** Try to cast this value to specified type `T`. If this object is not a value or it's not possible to cast this
        value to given type, return `fallback`. Supported all integer and floating-point types and std::string. */
    template <typename T>
    T cast(T fallback = {}) const;

    /** Return an iterator pointing to the first element in this object. */
    Iterator begin();
    ConstIterator begin() const;

    /** Return an iterator referring to the past-the-end element in this object. */
    Iterator end();
    ConstIterator end() const;

    /** Check whether this object is a dictionary or not. */
    bool is_dictionary() const;

    /** Check whether this object is a list or not. */
    bool is_list() const;

    /** Check whether this object is a value or not. */
    bool is_value() const;

    /** Check whether this object is valid or not. */
    bool is_valid() const;

private:
    typedef std::unordered_map<std::string, std::unique_ptr<Object>> Dictionary;
    typedef std::vector<std::unique_ptr<Object>> List;
    typedef std::string Value;

    std::variant<Dictionary, List, Value> m_value;
};

struct DeserializerContext {
    Deserializer& deserializer;
    World& world;
    entt::meta_handle component;
    entt::meta_handle property;
};

} // namespace hg
