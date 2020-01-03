#pragma once

#include "core/render/unique_handle.h"

namespace hg {

/** `HDRPassSingleComponent` contains HDR pass shaders, textures and uniforms. */
struct HDRPassSingleComponent final {
    UniqueHandle<bgfx::ProgramHandle> program;
    UniqueHandle<bgfx::UniformHandle> texture_sampler_uniform;
};

} // namespace hg
