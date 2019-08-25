#include "core/meta/registration.h"
#include "world/editor/editor_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<EditorComponent>("EditorComponent"_hs, std::make_pair("name"_hs, "EditorComponent"))
            .ctor<>()
            .data<&EditorComponent::name>("name"_hs, std::make_pair("name"_hs, "name"))
            .data<&EditorComponent::guid>("guid"_hs, std::make_pair("name"_hs, "guid"));
}

} // namespace hg
