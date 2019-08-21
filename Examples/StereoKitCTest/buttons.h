#pragma once

#include "../../StereoKitC/stereokit.h"

enum button_state_ {
	button_state_none = 0,
	button_state_pressed      = 1 << 0,
	button_state_justpressed  = 1 << 1,
	button_state_justreleased = 1 << 2,
};
SK_MakeFlag(button_state_);

struct button_t {
	transform_t base_tr;
	transform_t button_tr;
	solid_t base;
	solid_t button;
	joint_t slider;
	mesh_t  model;
	material_t base_mat;
	material_t button_mat;
	material_t button_active_mat;
	button_state_ state;
	vec3  press_pos;
	float press_dist;
};

button_t button_make(vec3 at, vec3 dir, vec3 dimensions, material_t base_mat, material_t button_mat, material_t button_active);
void     button_update(button_t &button);
void     button_destroy(button_t &button);

struct switch_t {
	transform_t tr;
	solid_t base_solid;
	solid_t switch_solid;
	joint_t switch_joint;
	mesh_t model;
	material_t mat;
};

switch_t switch_make(vec3 at, vec3 dir, vec3 dimensions, mesh_t model, material_t material);
void     switch_update(switch_t &sw);