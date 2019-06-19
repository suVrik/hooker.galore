#pragma once

#ifdef __MINGW64__
#include <filesystem>
#else
#include <ghc/filesystem.hpp>
#endif

namespace hg {

#ifdef __MINGW64__
namespace filesystem = std::filesystem;
#else
namespace filesystem = ghc::filesystem;
#endif

} // namespace hg
