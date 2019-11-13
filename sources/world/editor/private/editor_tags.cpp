#pragma once

#include "world/editor/editor_tags.h"
#include "world/imgui/imgui_tags.h"
#include "world/render/render_tags.h"

namespace hg::tags {

Tag editor("editor", render && imgui);

} // namespace hg::tags
