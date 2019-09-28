#pragma once

namespace hg {

/** Register all systems into `SystemManager`. */
void register_systems() noexcept;

/** Register all components into `ComponentManager`. */
void register_components() noexcept;

} // namespace hg
