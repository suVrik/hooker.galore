#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace hg {

class Object {
public:
    class Iterator;
    class ConstIterator;

    /** Return property with the specified `name`. If this object is not a dictionary, return invalid dictionary. */
    Object& operator[](const std::string& name);
    const Object& operator[](const std::string& name) const;

    /** Return element with the specified `index`. If this object is not a list, return invalid dictionary. */
    Object& operator[](size_t index);
    const Object& operator[](size_t index) const;
    
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
    struct Invalid {};

    typedef std::unordered_map<std::string, std::unique_ptr<Object>> Dictionary;
    typedef std::vector<std::unique_ptr<Object>> List;
    typedef std::string Value;

    std::variant<Invalid, Dictionary, List, Value> m_value;
};

class Object::Iterator {
public:
    /** For dictionary return name of specific property. For list return empty string. */
    const std::string& get_key() const;

    /** Return value of dictionary property or list element. */
    Object& get_value();
    const Object& get_value() const;

private:
    Iterator() = default;
    Iterator(Dictionary::iterator value);
    Iterator(List::iterator value);

    std::variant<Invalid, Dictionary::iterator, List::iterator> m_value;

    friend class Object;
};
    
class Object::ConstIterator {
public:
    /** For dictionary return name of specific property. For list return empty string. */
    const std::string& get_key() const;

    /** Return value of dictionary property or list element. */
    const Object& get_value() const;

private:
    ConstIterator() = default;
    ConstIterator(Dictionary::const_iterator value);
    ConstIterator(List::const_iterator value);

    std::variant<Invalid, Dictionary::const_iterator, List::const_iterator> m_value;

    friend class Object;
};

Object::Iterator::Iterator(Dictionary::iterator value) 
        : m_value(value) {
}

Object::Iterator::Iterator(List::iterator value)
        : m_value(value) {
}

const std::string& Object::Iterator::get_key() const {
    if (std::holds_alternative<Dictionary::iterator>(m_value)) {
        return std::get<Dictionary::iterator>(m_value)->first;
    }

    static const std::string EMPTY;
    return EMPTY;
}

Object& Object::Iterator::get_value() {
    if (std::holds_alternative<Dictionary::iterator>(m_value)) {
        return *std::get<Dictionary::iterator>(m_value)->second;
    }
    assert(std::holds_alternative<List::iterator>(m_value));
    return **std::get<List::iterator>(m_value);
}

const Object& Object::Iterator::get_value() const {
    return const_cast<Object::Iterator*>(this)->get_value();
}

Object::ConstIterator::ConstIterator(Dictionary::const_iterator value)
        : m_value(value) {
}

Object::ConstIterator::ConstIterator(List::const_iterator value)
        : m_value(value) {
}

const std::string& Object::ConstIterator::get_key() const {
    if (std::holds_alternative<Dictionary::const_iterator>(m_value)) {
        return std::get<Dictionary::const_iterator>(m_value)->first;
    }

    static const std::string EMPTY;
    return EMPTY;
}

const Object& Object::ConstIterator::get_value() const {
    if (std::holds_alternative<Dictionary::const_iterator>(m_value)) {
        return *std::get<Dictionary::const_iterator>(m_value)->second;
    }
    assert(std::holds_alternative<List::const_iterator>(m_value));
    return **std::get<List::const_iterator>(m_value);
}


Object& Object::operator[](const std::string& name) {
    if (std::holds_alternative<Dictionary>(m_value)) {
        std::unique_ptr<Object>& result = std::get<Dictionary>(m_value)[name];
        if (!result) {
            result = std::make_unique<Object>();
        }
        return *result;
    }

    static Object INVALID;
    return INVALID;
}

const Object& Object::operator[](const std::string& name) const {
    return (*const_cast<Object*>(this))[name];
}

Object& Object::operator[](size_t index) {
    if (std::holds_alternative<Dictionary>(m_value)) {
        std::unique_ptr<Object>& result = std::get<Dictionary>(m_value)[name];
        if (!result) {
            result = std::make_unique<Object>();
        }
        return *result;
    }

    static Object INVALID;
    return INVALID;
}

const Object& Object::operator[](size_t index) const {
    return (*const_cast<Object*>(this))[index];
}

size_t Object::get_size() const {
    size_t result;

    std::visit([&](auto&& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, Invalid>) {
            result = 0;
        } else if constexpr (std::is_same_v<T, Dictionary> || std::is_same_v<T, List>) {
            result = value.size();
        } else if constexpr (std::is_same_v<T, Value>) {
            result = 1;
        } else {
            static_assert(false);
        }
    }, m_value);

    return result;
}

Object::Iterator Object::begin() {
    if (std::holds_alternative<Dictionary>(m_value)) {
        return Iterator(std::get<Dictionary>(m_value).begin());
    } else if (std::holds_alternative<List>(m_value)) {
        return Iterator(std::get<List>(m_value).begin());
    }
    return Iterator();
}

Object::ConstIterator Object::begin() const {
    if (std::holds_alternative<Dictionary>(m_value)) {
        return ConstIterator(std::get<Dictionary>(m_value).cbegin());
    } else if (std::holds_alternative<List>(m_value)) {
        return ConstIterator(std::get<List>(m_value).cbegin());
    }
    return ConstIterator();
}

Object::Iterator Object::end() {
    if (std::holds_alternative<Dictionary>(m_value)) {
        return Iterator(std::get<Dictionary>(m_value).end());
    } else if (std::holds_alternative<List>(m_value)) {
        return Iterator(std::get<List>(m_value).end());
    }
    return Iterator();
}

Object::ConstIterator Object::end() const {
    if (std::holds_alternative<Dictionary>(m_value)) {
        return ConstIterator(std::get<Dictionary>(m_value).cend());
    } else if (std::holds_alternative<List>(m_value)) {
        return ConstIterator(std::get<List>(m_value).cend());
    }
    return ConstIterator();
}

bool Object::is_dictionary() const {
    return std::holds_alternative<Dictionary>(m_value);
}

bool Object::is_list() const {
    return std::holds_alternative<List>(m_value);
}

bool Object::is_value() const {
    return std::holds_alternative<Value>(m_value);
}

bool Object::is_valid() const {
    return !std::holds_alternative<Invalid>(m_value);
}

} // namespace hg
