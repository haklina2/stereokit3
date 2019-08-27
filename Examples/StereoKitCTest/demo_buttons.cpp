#include "demo_buttons.h"

#include "../../StereoKitC/stereokit.h"
#include "buttons.h"

#include <vector>
using namespace std;

///////////////////////////////////////////

vector<solid_t> scene_objects;
joint_t         grabbed_objects[2];
model_t         object_model;

transform_t dbt_floor_tr;
material_t  dbt_floor_mat;
model_t     dbt_floor_model;
solid_t     dbt_floor_solid;
transform_t viewpt;

button_t button_reset;
button_t button_spawn;

material_t hand_ghost_mat;
material_t default_mat;
material_t button_active_mat;
material_t button_idle_mat;

void demo_buttons_init();
void demo_buttons_update();
void demo_buttons_shutdown();
void demo_buttons_clear_objs();

///////////////////////////////////////////

void demo_buttons_init() {
	// Initialize the assets

	// Set up a cubemap
	tex2d_t cubemap = tex2d_create_cubemap_file("../../Examples/Assets/Sky/sky.hdr");
	render_set_skytex(cubemap, true);
	tex2d_release(cubemap);

	default_mat = material_find("default/material");
	material_set_float(default_mat, "metallic", 0);

	hand_ghost_mat = material_copy("app/hand_mat", default_mat);
	material_set_color32(hand_ghost_mat, "color", { 255,255,255,128 });
	material_set_alpha_mode(hand_ghost_mat, material_alpha_blend);

	// Create a PBR floor material
	tex2d_t tex_color = tex2d_create_file("../../Examples/Assets/test.png");
	tex2d_t tex_norm  = tex2d_create_file("../../Examples/Assets/test_normal.png");
	dbt_floor_mat = material_create("app/material_floor", shader_find("default/shader_pbr"));
	material_set_texture(dbt_floor_mat, "diffuse", tex_color);
	material_set_texture(dbt_floor_mat, "normal",  tex_norm);
	material_set_float  (dbt_floor_mat, "tex_scale", 6);
	material_set_float  (dbt_floor_mat, "roughness", 1.0f);
	material_set_float  (dbt_floor_mat, "metallic", 0.5f);
	material_set_queue_offset(dbt_floor_mat, 1);
	if (tex_color != nullptr) tex2d_release(tex_color);
	if (tex_norm  != nullptr) tex2d_release(tex_norm);

	// Procedurally create a cube model
	mesh_t mesh_cube = mesh_gen_cube("app/mesh_cube", vec3_one);
	dbt_floor_model = model_create_mesh("app/model_cube", mesh_cube, dbt_floor_mat);
	mesh_release(mesh_cube);

	// Load a gltf model
	object_model = model_create_file("../../Examples/Assets/DamagedHelmet.gltf");

	// Set hand materials
	button_active_mat = material_copy("app/button_active", default_mat);
	material_set_alpha_mode(button_active_mat, material_alpha_blend);
	button_idle_mat   = material_copy("app/button_idle",   button_active_mat);
	material_set_color(button_active_mat, "color", { 1.f, 0.6f, 0.6f, 0.8f });
	material_set_color(button_idle_mat,   "color", { 0.6f, 1.f, 0.6f, 0.8f });
	
	// Initialize the scene

	// Make some buttons
	button_spawn = button_make({  .3f,0, 0.3f }, {  1, 1, 1 }, vec3{ 10, 10, 4 } * cm2m, default_mat, button_idle_mat, button_active_mat);
	button_reset = button_make({ -.3f,0,-0.3f }, { -1, 1,-1 }, vec3{ 10, 10, 4 } * cm2m, default_mat, button_idle_mat, button_active_mat);
	//sw           = switch_make({ .25f,0,0.35f }, { 1,1,1 }, { 0.04f, 0.04f, 0.1f }, mesh_cube, def);

	// Build a physical floor!
	transform_set(dbt_floor_tr, { 0,-1.5f,0 }, vec3{ 5,1,5 }, quat_identity);
	dbt_floor_solid = solid_create(dbt_floor_tr._position, dbt_floor_tr._rotation, solid_type_immovable);
	solid_add_box (dbt_floor_solid, dbt_floor_tr._scale);

	// Set the camera viewpoint
	transform_initialize  (viewpt);
	transform_set_position(viewpt, vec3_one*0.4f);
	transform_lookat      (viewpt, vec3_zero);
	render_set_view       (viewpt);
}

///////////////////////////////////////////

void demo_buttons_shutdown() {
	// Release scene
	demo_buttons_clear_objs();
	button_destroy(button_reset);
	button_destroy(button_spawn);
	solid_release(dbt_floor_solid);

	// Release assets
	model_release(object_model);
	model_release(dbt_floor_model);

	material_release(dbt_floor_mat);
	material_release(hand_ghost_mat);
	material_release(default_mat);
	material_release(button_active_mat);
	material_release(button_idle_mat);
}

///////////////////////////////////////////

void demo_buttons_update() {
	button_update(button_spawn);
	button_update(button_reset);

	// Check for button input
	if (button_spawn.state & button_state_justpressed) {
		solid_t new_obj = solid_create({ 0,3,0 }, quat_identity);
		solid_add_sphere(new_obj, 0.45f, 40);
		solid_add_box   (new_obj, vec3_one*0.35f, 40);
		scene_objects.push_back(new_obj);
	}
	if (button_reset.state & button_state_justpressed) {
		demo_buttons_clear_objs();
	}

	// Do hand input
	for (size_t i = 0; i < handed_max; i++) {
		bool r_solid = input_hand((handed_)i).state & input_state_grip;
		input_hand_material((handed_)i, r_solid ? default_mat : hand_ghost_mat);
		input_hand_solid   ((handed_)i, r_solid);

		// If they grab something, join it to the hand
		if (input_hand((handed_)i).state & input_state_justpinch) {
			solid_t s = physics_get_at(input_hand((handed_)i).root.position);
			if (s != nullptr) {
				grabbed_objects[i] = joint_make_fixed(input_hand((handed_)i).root_solid, s);
			}
		}
		// If they release something, get rid of any joint attached
		if (input_hand((handed_)i).state & input_state_unpinch) {
			if (grabbed_objects[i] != nullptr)
				joint_release(grabbed_objects[i]);
			grabbed_objects[i] = nullptr;
		}
	}

	// Render spawned objects
	transform_t tr;
	transform_set_scale(tr, vec3_one*0.25f);
	for (size_t i = 0; i < scene_objects.size(); i++) {
		solid_get_transform(scene_objects[i], tr);
		render_add_model   (object_model, tr);
	}

	// Render floor
	render_add_model(dbt_floor_model, dbt_floor_tr);
}

///////////////////////////////////////////

void demo_buttons_clear_objs() {
	for (size_t i = 0; i < handed_max; i++) {
		if (grabbed_objects[i] != nullptr) joint_release(grabbed_objects[i]);
		grabbed_objects[i] = nullptr;
	}
	for (size_t i = 0; i < scene_objects.size(); i++)
		solid_release(scene_objects[i]);
	scene_objects.clear();
}