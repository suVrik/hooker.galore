#ifndef ENTT_META_POLICY_HPP
#define ENTT_META_POLICY_HPP


namespace entt {


/*! @brief Empty class type used to request the _as alias_ policy. */
struct as_alias_t {};


/*! @brief Disambiguation tag. */
constexpr as_alias_t as_alias;


/*! @brief Empty class type used to request the _as-is_ policy. */
struct as_is_t {};


/*! @brief Empty class type used to request the _as void_ policy. */
struct as_void_t {};


/*! @brief Makes `meta_handle` constructed from `meta_handle` a nested `meta_handle`, rather than a copy of a `meta_handle`. */
struct dont_merge_t {};

}


#endif // ENTT_META_POLICY_HPP
