#pragma once

#include "world/resource/resource_geometry.h"

#include <cassert>
#include <memory>

namespace hg {

/** `ResourceGeometryComponent` contains render geometry. Entity with this component will be processed by geometry pass
    system, shadow pass system and other render systems that require geometry. */
class ResourceGeometryComponent {
public:
    /** Get/Set entity's render geometry. */
    const std::shared_ptr<ResourceGeometry>& get_geometry() const;
    void set_geometry(const std::shared_ptr<ResourceGeometry>& geometry);

private:
    std::shared_ptr<ResourceGeometry> m_geometry;
};

} // namespace hg

#include "world/resource/private/resource_geometry_component_impl.h"
