#include "core/ecs/tags.h"

#include <cassert>

namespace hg {

std::vector<Tag::TagDescriptor> Tag::descriptors;

size_t Tag::get_tags_count() {
    return descriptors.size();
}

Tag Tag::get_tag_by_index(size_t index) {
    assert(index < descriptors.size());
    return Tag(index);
}

Tag::Tag(const std::string& name, bool is_inheritable, bool is_propagable) 
        : m_index(descriptors.size()) {
    descriptors.push_back(TagDescriptor{ name, is_inheritable, is_propagable });
    assert(m_index + 1 == descriptors.size());
}

Tag::Tag(size_t index) 
        : m_index(index) {
    assert(m_index < descriptors.size());
}

size_t Tag::get_index() const {
    assert(m_index < descriptors.size());
    return m_index;
}

const std::string& Tag::get_name() const {
    assert(m_index < descriptors.size());
    return descriptors[m_index].name;
}

bool Tag::is_inheritable() const {
    assert(m_index < descriptors.size());
    return descriptors[m_index].is_inheritable;
}

bool Tag::is_propagable() const {
    assert(m_index < descriptors.size());
    return descriptors[m_index].is_propagable;
}

} // namespace hg
