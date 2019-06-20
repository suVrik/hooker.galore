#include "core/meta/registration.h"
#include "world/editor/editor_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<EditorComponent>("EditorComponent"_hs)
            .ctor<>()
            .data<&EditorComponent::name>("name"_hs)
            .data<&EditorComponent::guid>("guid"_hs);
}

} // namespace hg
