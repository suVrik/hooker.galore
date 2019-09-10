#include "core/meta/registration.h"
#include "world/editor/editor_component.h"

namespace hg {

REFLECTION_REGISTRATION {
    entt::reflect<EditorComponent>("EditorComponent")
            .data<&EditorComponent::name>("name")
            .data<&EditorComponent::guid>("guid");
}

} // namespace hg
