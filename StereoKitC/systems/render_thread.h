#pragma once

#include "render.h"

namespace sk {

void render_thread_begin();
void render_thread_submit(render_list_t render_list);
void render_thread_end();

}