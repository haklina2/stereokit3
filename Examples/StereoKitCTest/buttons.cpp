#pragma once
#include "buttons.h"

button_t button_make(vec3 at, vec3 dir, vec3 dimensions, material_t base_mat, material_t button_mat, material_t button_active) {
	button_t result = {};
	float radius = fminf(fminf(dimensions.x, dimensions.y), dimensions.z);
	result.model      = mesh_gen_rounded_cube("app/mesh_btn", dimensions, radius*0.1f, 2);
	result.button_mat = button_mat;
	result.base_mat   = base_mat;
	result.button_active_mat = button_active;

	// |     size      |
	// |back |pressable|
	// |-----|---------|
	// | .25 |   .75   |
	// |  .  |    .    | centers
	// .               | at

	float back_size  = dimensions.z * 0.25f;
	float press_size = dimensions.z * 0.75f;
	vec3  normal     = vec3_normalize(dir);
	vec3  back_pos   = at + normal * (back_size / 2);
	vec3  press_pos  = at + normal*back_size + normal*(press_size / 2);

	result.press_dist = (press_size / 2) * 0.6f;
	result.press_pos  = press_pos;

	transform_set   (result.base_tr, back_pos, { 1, 1, 0.25f }, { 0,0,0,1 });
	transform_lookat(result.base_tr, dir + at);
	transform_set   (result.button_tr, press_pos, { 1, 1, 0.75f}, { 0,0,0,1 });
	transform_lookat(result.button_tr, dir + at);

	result.base   = solid_create(result.base_tr  ._position, result.base_tr  ._rotation, solid_type_immovable);
	result.button = solid_create(result.button_tr._position, result.button_tr._rotation);
	solid_add_box(result.button, result.button_tr._scale * dimensions, 40);
	solid_set_gravity(result.button, false);

	result.slider = joint_make_slider( result.base, result.button, 20, 100, -press_size, 0.001f);
	return result;
}
void     button_update(button_t &button) {
	solid_get_transform(button.base,   button.base_tr);
	solid_get_transform(button.button, button.button_tr);

	bool  was_pressed     = button.state & button_state_pressed;
	float activation_dist = was_pressed ? button.press_dist*0.75f : button.press_dist;
	float dist            = vec3_magnitude_sq( button.button_tr._position - button.press_pos );
	bool  is_pressed      = dist > activation_dist * activation_dist;
	button.state = is_pressed ? button_state_pressed : button_state_none;
	if (was_pressed && !is_pressed)
		button.state |= button_state_justreleased;
	if (!was_pressed && is_pressed)
		button.state |= button_state_justpressed;

	render_add_mesh(button.model, button.base_mat,   button.base_tr);
	render_add_mesh(button.model, is_pressed? button.button_active_mat : button.button_mat, button.button_tr);
}
void button_destroy(button_t &button) {
	joint_release(button.slider);
	joint_release(button.base);
	joint_release(button.button);
	mesh_release(button.model);
}


switch_t switch_make(vec3 at, vec3 dir, vec3 dimensions, mesh_t model, material_t material) {
	switch_t result = {};
	result.model = model;
	result.mat   = material;

	vec3 normal = vec3_normalize(dir);
	transform_set   (result.tr, at, dimensions, { 0,0,0,1 });
	transform_lookat(result.tr, at + dir);
	result.base_solid   = solid_create(at, result.tr._rotation, solid_type_immovable);
	result.switch_solid = solid_create(at+normal * dimensions.z, result.tr._rotation);

	result.switch_joint = joint_make_hinge(result.base_solid, result.switch_solid);

	return result;
}
void     switch_update(switch_t &sw) {
	solid_get_transform(sw.switch_solid, sw.tr);

	render_add_mesh(sw.model, sw.mat, sw.tr);
}