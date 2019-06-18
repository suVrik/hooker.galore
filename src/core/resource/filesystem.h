#pragma once

#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && __has_include(<filesystem>)
#include <filesystem>
#else
#include <ghc/filesystem.hpp>
#endif

namespace hg {

#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include) && __has_include(<filesystem>)
namespace filesystem = std::filesystem;
#else
namespace filesystem = ghc::filesystem;
#endif

} // namespace hg
