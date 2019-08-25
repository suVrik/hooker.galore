#include "world/shared/name_single_component.h"

namespace hg {

std::string NameSingleComponent::acquire_unique_name(entt::entity entity, const std::string& prototype) noexcept {
#ifndef NDEBUG
    for (auto& [name, another_entity] : name_to_entity) {
        assert(entity != another_entity && "Unregister the previous name first.");
    }
#endif

    if (name_to_entity.count(prototype) > 0) {
        auto find_similar_name = [&](const std::string& original) {
            size_t index = 2;
            std::string similar_name = original + "-" + std::to_string(index);
            while (name_to_entity.count(similar_name) > 0) {
                similar_name = original + "-" + std::to_string(++index);
            }
            name_to_entity[similar_name] = entity;
            return similar_name;
        };

        auto dash_position = prototype.find_last_of('-');
        if (dash_position == std::string::npos || dash_position == 0 || dash_position + 1 == prototype.size()) {
            return find_similar_name(prototype);
        } else {
            bool is_number = true;
            for (size_t i = dash_position + 1; i < prototype.size(); i++) {
                if (!std::isdigit(prototype[i])) {
                    is_number = false;
                    break;
                }
            }

            if (is_number) {
                return find_similar_name(prototype.substr(0, dash_position));
            } else {
                return find_similar_name(prototype);
            }
        }
    } else {
        name_to_entity[prototype] = entity;
        return prototype;
    }
}

} // namespace hg
