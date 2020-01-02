#pragma once

#include <entt/entity/registry.hpp>
#include <entt/meta/meta.hpp>
#include <unordered_map>

namespace hg {

class World;

/** `ComponentManager` is an interface to manipulate over components on runtime. */
class ComponentManager {
public:
    ComponentManager() = delete;

    /** Register component type `T` so `ComponentManager` and `World` member functions expecting `entt::meta_type`
        and `entt::meta_handle` can be applied to components of this type. */
    template <typename T>
    static void register_component();

    /** Iterate over all registered component types.

        ComponentManager::each_registered([](const entt::meta_type component_type) {
            // Your code goes here
        }); */
    template <typename T>
    static void each_registered(T callback);

    /** Iterate over all editable component types.

        ComponentManager::each_editable([](const entt::meta_type component_type) {
            // Your code goes here
        }); */
    template <typename T>
    static void each_editable(T callback);

    /** Construct specified component using default constructor. Component must be default-constructible. */
    static entt::meta_any construct(entt::meta_type component_type);

    /** Acquire copy of specified component using copy constructor. Component must be copy-constructible. */
    static entt::meta_any copy(entt::meta_handle component);

    /** Acquire copy of specified component using move constructor. Component must be move-constructible. */
    static entt::meta_any move(entt::meta_handle component);

    /** Acquire copy of specified component using move or copy constructor. Component must be move-constructible or copy-constructible. */
    static entt::meta_any move_or_copy(entt::meta_handle component);

    /** Return name of specified component. */
    static const char* get_name(entt::meta_type component_type, const char* fallback = "undefined");

    /** Check whether specified component type is registered. */
    static bool is_registered(entt::meta_type component_type);

    /** Check whether component type is default constructible. */
    static bool is_default_constructible(entt::meta_type component_type);

    /** Check whether component type is copy constructible. */
    static bool is_copy_constructible(entt::meta_type component_type);

    /** Check whether component type is move constructible. */
    static bool is_move_constructible(entt::meta_type component_type);

    /** Check whether component type is copy assignable. */
    static bool is_copy_assignable(entt::meta_type component_type);

    /** Check whether component type is copy assignable. */
    static bool is_move_assignable(entt::meta_type component_type);

    /** Check whether the specified component is ignored. */
    static bool is_ignored(entt::meta_type component_type);

    /** Check whether the specified component is editable. Editable component is a non-ignored component with valid
        name, default constructor, copy constructor and copy assignment operator. Only editable components can be
        modified in property editor. Only editable components are copied when entity, they're assigned to, is copied.
        Only editable components are serialized and deserialized from level and preset files. */
    static bool is_editable(entt::meta_type component_type);

private:
    struct ComponentDescriptor {
        entt::meta_any(*construct)();
        entt::meta_any(*copy)(entt::meta_handle component);
        entt::meta_any(*move)(entt::meta_handle component);
        entt::meta_handle(*set)(entt::registry* registry);
        void(*unset)(entt::registry* registry);
        entt::meta_handle(*ctx)(const entt::registry* registry);
        entt::meta_handle(*assign_default)(entt::registry* registry, entt::entity entity);
        entt::meta_handle(*assign_copy)(entt::registry* registry, entt::entity entity, entt::meta_handle component);
        entt::meta_handle(*assign_move)(entt::registry* registry, entt::entity entity, entt::meta_handle component);
        entt::meta_handle(*replace_copy)(entt::registry* registry, entt::entity entity, entt::meta_handle component);
        entt::meta_handle(*replace_move)(entt::registry* registry, entt::entity entity, entt::meta_handle component);
        void(*remove)(entt::registry* registry, entt::entity entity);
        bool(*has)(const entt::registry* registry, entt::entity entity);
        entt::meta_handle(*get)(const entt::registry* registry, entt::entity entity);
        entt::meta_handle(*get_or_assign)(entt::registry* registry, entt::entity entity);
    };

    static std::unordered_map<entt::meta_type, ComponentDescriptor> descriptors;

    friend class World;
};

} // namespace hg

#include "core/ecs/private/component_manager_impl.h"
