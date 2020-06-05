#pragma once

#include "../render.h"

namespace sk {

bool platform_init();
void platform_shutdown();
void platform_begin_frame();
void platform_end_frame();
void platform_present();
void platform_render(render_list_t render_list);

} // namespace sk