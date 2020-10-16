#pragma once

#include "world/resource/resource_geometry_component.h"

namespace hg {

inline const std::shared_ptr<ResourceGeometry>& ResourceGeometryComponent::get_geometry() const {
    return m_geometry;
}

inline void ResourceGeometryComponent::set_geometry(const std::shared_ptr<ResourceGeometry>& geometry) {
    m_geometry = geometry;
}

} // namespace hg
