#include "../../StereoKitC/stereokit.h"

#include <vector>
using namespace std;
vector<solid_t> phys_objs;

transform_t floor_tr;
transform_t tr;
model_t     gltf;
model_t     box;

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
	mesh_t  model;
	material_t base_mat;
	material_t button_mat;
	material_t button_active_mat;
	button_state_ state;
	vec3  press_pos;
	float press_dist;
};
button_t button_make(vec3 at, vec3 dir, vec3 dimensions, mesh_t model, material_t base_mat, material_t button_mat, material_t button_active) {
	button_t result = {};
	result.model      = model;
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

	transform_set   (result.base_tr, back_pos, { dimensions.x, dimensions.y, back_size }, { 0,0,0,1 });
	transform_lookat(result.base_tr, dir + at);
	transform_set   (result.button_tr, press_pos, { dimensions.x, dimensions.y, press_size }, { 0,0,0,1 });
	transform_lookat(result.button_tr, dir + at);

	result.base   = solid_create(result.base_tr  ._position, result.base_tr  ._rotation, solid_type_immovable);
	result.button = solid_create(result.button_tr._position, result.button_tr._rotation);
	solid_add_box(result.button, result.button_tr._scale, 40);
	solid_set_gravity(result.button, false);

	solid_add_joint( result.base, result.button );
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

struct switch_t {
	transform_t tr;
	solid_t base_solid;
	solid_t switch_solid;
	mesh_t model;
	material_t mat;
};
switch_t switch_make(vec3 at, vec3 dir, vec3 dimensions, mesh_t model, material_t material) {
	switch_t result = {};
	result.model = model;
	result.mat   = material;

	vec3 normal = vec3_normalize(dir);
	transform_set   (result.tr, at, dimensions, { 0,0,0,1 });
	transform_lookat(result.tr, at + dir);
	result.base_solid   = solid_create(at, result.tr._rotation, solid_type_immovable);
	result.switch_solid = solid_create(at+normal * dimensions.z, result.tr._rotation);
	
	solid_add_joint2(result.base_solid, result.switch_solid);

	return result;
}
void     switch_update(switch_t &sw) {
	solid_get_transform(sw.switch_solid, sw.tr);

	render_add_mesh(sw.model, sw.mat, sw.tr);
}

button_t button_reset;
button_t button;
switch_t sw;

int main() {
	if (!sk_init("StereoKit C", sk_runtime_mixedreality))
		return 1;

	const char *cube_files[] = {
		"../../Examples/Assets/Sky/Right.jpg",
		"../../Examples/Assets/Sky/Left.jpg",
		"../../Examples/Assets/Sky/Top.jpg",
		"../../Examples/Assets/Sky/Bottom.jpg",
		"../../Examples/Assets/Sky/Back.jpg",
		"../../Examples/Assets/Sky/Front.jpg",};
	tex2d_t cubemap = tex2d_create_cubemap_files(cube_files);
	render_set_skytex(cubemap, true);
	tex2d_release(cubemap);

	material_t def = material_find("default/material");
	material_set_float(def, "metallic", 0);

	// Create a PBR floor material
	tex2d_t    tex_color = tex2d_create_file("../../Examples/Assets/test.png");
	tex2d_t    tex_norm  = tex2d_create_file("../../Examples/Assets/test_normal.png");
	material_t floor_mat = material_create("app/material_floor", shader_find("default/shader_pbr"));
	material_set_texture(floor_mat, "diffuse", tex_color);
	material_set_texture(floor_mat, "normal",  tex_norm);
	material_set_float  (floor_mat, "tex_scale", 6);
	material_set_float  (floor_mat, "roughness", 1.0f);
	material_set_float  (floor_mat, "metallic", 0.5f);
	material_set_queue_offset(floor_mat, 1);
	if (tex_color != nullptr) tex2d_release(tex_color);
	if (tex_norm  != nullptr) tex2d_release(tex_norm);

	// Procedurally create a cube model
	mesh_t mesh_cube = mesh_gen_cube("app/mesh_cube", { 1,1,1 }, 0);
	box  = model_create_mesh("app/model_cube", mesh_cube, floor_mat);
	mesh_release(mesh_cube);

	// Load a gltf model
	gltf = model_create_file("../../Examples/Assets/DamagedHelmet.gltf");

	// Build a physical floor!
	transform_set(floor_tr, { 0,-1.5f,0 }, vec3{ 5,1,5 }, { 0,0,0,1 });
	solid_t floor = solid_create(floor_tr._position, floor_tr._rotation, solid_type_immovable);
	solid_add_box (floor, floor_tr._scale);

	material_t m_active = material_copy("app/button_active", def);
	material_set_alpha_mode(m_active, material_alpha_blend);
	material_t m_idle   = material_copy("app/button_idle",   m_active);
	material_set_color(m_active, "color", { 1.f, 0.6f, 0.6f, 0.8f });
	material_set_color(m_idle,   "color", { 0.6f, 1.f, 0.6f, 0.8f });

	button       = button_make({ .3f,0,0.3f },   { 1,1,1 },   { 0.1, 0.1, 0.04f }, mesh_cube, def, m_idle, m_active);
	button_reset = button_make({ -.3f,0,-0.3f }, { -1,1,-1 }, { 0.1, 0.1, 0.04f }, mesh_cube, def, m_idle, m_active);
	sw = switch_make({ .25f,0,0.35f }, { 1,1,1 }, { 0.04f, 0.04f, 0.1f }, mesh_cube, def);
	
	transform_t viewpt;
	transform_initialize(viewpt);
	transform_set_position(viewpt, vec3{ 1,1,1 } *0.4f);
	transform_lookat(viewpt, { 0,0,0 });
	render_set_view(viewpt);

	while (sk_step( []() {
		// update button!
		button_update(button);
		button_update(button_reset);
		switch_update(sw);

		// Do hand input
		//if (input_hand(handed_right).state & input_state_justpinch) {
		if (button.state & button_state_justpressed) {
			solid_t new_obj = solid_create({ 0,3,0 }, { 0,0,0,1 });
			solid_add_sphere(new_obj, 0.45f, 40);
			solid_add_box   (new_obj, vec3{1,1,1}*0.35f, 40);
			phys_objs.push_back(new_obj);
		}
		if (button_reset.state & button_state_justpressed) {
			for (size_t i = 0; i < phys_objs.size(); i++)
				solid_release(phys_objs[i]);
			phys_objs.clear();
		}

		// Render solid helmets
		transform_set_scale(tr, vec3{ 1,1,1 }*0.25f);
		for (size_t i = 0; i < phys_objs.size(); i++) {
			solid_get_transform(phys_objs[i], tr);
			render_add_model   (gltf, tr);
		}
		
		// Render floor
		render_add_model(box, floor_tr);
	}));

	// Release everything
	for (size_t i = 0; i < phys_objs.size(); i++)
		solid_release(phys_objs[i]);
	solid_release(floor);
	model_release(gltf);
	material_release(floor_mat);
	model_release(box);

	sk_shutdown();
	return 0;
}