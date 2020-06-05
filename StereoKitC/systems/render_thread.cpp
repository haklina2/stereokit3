#include "render_thread.h"

#define THREAD_IMPLEMENTATION
#define THREAD_U64 uint64_t
#include "../libraries/thread.h"

#include "platform/platform.h"



namespace sk {

///////////////////////////////////////////
	
struct render_view_t {
	array_t<matrix> views;
	array_t<matrix> projections;
	tex_t           target;
};
struct render_views_t {
	render_list_t          list;
	array_t<render_view_t> views;
};

thread_ptr_t  render_thread_id;
bool32_t      render_thread_run   = false;
render_list_t render_thread_curr = nullptr;
render_list_t render_thread_next = nullptr;

///////////////////////////////////////////

int32_t render_thread(void *data);

///////////////////////////////////////////

void render_thread_begin() {
	log_info("thread create");
	render_thread_run = true;
	render_thread_id = thread_create(render_thread, nullptr, "render_thread", THREAD_STACK_SIZE_DEFAULT ); 
}

///////////////////////////////////////////

void render_thread_submit(render_list_t render_list) {
	if (render_thread_next != nullptr) {
		render_list_t tmp = render_thread_next;
		render_thread_next = nullptr;
		render_list_clear(tmp);
	}
	render_thread_next = render_list;
}

///////////////////////////////////////////

void render_thread_end() {
	render_thread_run = false;
	thread_join   ( render_thread_id );
	thread_destroy( render_thread_id );
}

///////////////////////////////////////////

int32_t render_thread(void *data) {
	
	while (render_thread_run) {
		
		if (render_thread_next != nullptr) {
			if (render_thread_curr != nullptr) {
				render_list_clear(render_thread_curr);
			}
			render_thread_curr = render_thread_next;
			render_thread_next = nullptr;
		}

		if (render_thread_curr != nullptr) {
			platform_render(render_thread_curr);
		}
	}
	return 0;
}

}