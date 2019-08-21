#include "../../StereoKitC/stereokit.h"
#include "buttons.h"

#include <vector>
using namespace std;

vector<solid_t> scene_objects;
joint_t         grabbed_objects[2];
model_t         object_model;

transform_t floor_tr;
material_t  floor_mat;
model_t     floor_model;
solid_t     floor_solid;
transform_t viewpt;

button_t button_reset;
button_t button_spawn;

material_t hand_ghost_mat;
material_t hand_solid_mat;
material_t default_mat;
material_t button_active_mat;
material_t button_idle_mat;

void load_assets();
void release_assets();
void scene_setup();
void scene_shutdown();
void clear_objects();

int main() {
	if (!sk_init("StereoKit C", sk_runtime_flatscreen))
		return 1;

	load_assets();
	scene_setup();

	while (sk_step( []() {
		// update button!
		button_update(button_spawn);
		button_update(button_reset);

		// Check for button input
		if (button_spawn.state & button_state_justpressed) {
			solid_t new_obj = solid_create({ 0,3,0 }, { 0,0,0,1 });
			solid_add_sphere(new_obj, 0.45f, 40);
			solid_add_box   (new_obj, vec3{1,1,1}*0.35f, 40);
			scene_objects.push_back(new_obj);
		}
		if (button_reset.state & button_state_justpressed) {
			clear_objects();
		}
		
		// Do hand input
		for (size_t i = 0; i < handed_max; i++) {
			bool r_solid = input_hand((handed_)i).state & input_state_grip;
			input_hand_material((handed_)i, r_solid ? hand_solid_mat : hand_ghost_mat);
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
		transform_set_scale(tr, vec3{ 1,1,1 }*0.25f);
		for (size_t i = 0; i < scene_objects.size(); i++) {
			solid_get_transform(scene_objects[i], tr);
			render_add_model   (object_model, tr);
		}
		
		// Render floor
		render_add_model(floor_model, floor_tr);
	}));

	scene_shutdown();
	release_assets();

	sk_shutdown();
	return 0;
}

void scene_setup() {
	// Make some buttons
	button_spawn = button_make({ .3f,0,0.3f },   { 1,1,1 },   { 0.1, 0.1, 0.04f }, default_mat, button_idle_mat, button_active_mat);
	button_reset = button_make({ -.3f,0,-0.3f }, { -1,1,-1 }, { 0.1, 0.1, 0.04f }, default_mat, button_idle_mat, button_active_mat);
	//sw           = switch_make({ .25f,0,0.35f }, { 1,1,1 }, { 0.04f, 0.04f, 0.1f }, mesh_cube, def);

	// Build a physical floor!
	transform_set(floor_tr, { 0,-1.5f,0 }, vec3{ 5,1,5 }, { 0,0,0,1 });
	floor_solid = solid_create(floor_tr._position, floor_tr._rotation, solid_type_immovable);
	solid_add_box (floor_solid, floor_tr._scale);

	// Set the camera viewpoint
	transform_initialize  (viewpt);
	transform_set_position(viewpt, vec3{ 1,1,1 } *0.4f);
	transform_lookat      (viewpt, { 0,0,0 });
	render_set_view       (viewpt);
}

void clear_objects() {
	for (size_t i = 0; i < handed_max; i++) {
		if (grabbed_objects[i] != nullptr) joint_release(grabbed_objects[i]);
		grabbed_objects[i] = nullptr;
	}
	for (size_t i = 0; i < scene_objects.size(); i++)
		solid_release(scene_objects[i]);
	scene_objects.clear();
}

void scene_shutdown() {
	clear_objects();
	button_destroy(button_reset);
	button_destroy(button_spawn);
	solid_release(floor_solid);
}

void load_assets() {
	// Set up a cubemap
	const char *cube_files[] = {
		"../../Examples/Assets/Sky/Right.jpg", "../../Examples/Assets/Sky/Left.jpg",
		"../../Examples/Assets/Sky/Top.jpg",   "../../Examples/Assets/Sky/Bottom.jpg",
		"../../Examples/Assets/Sky/Back.jpg",  "../../Examples/Assets/Sky/Front.jpg",};
	tex2d_t cubemap = tex2d_create_cubemap_files(cube_files);
	render_set_skytex(cubemap, true);
	tex2d_release(cubemap);

	default_mat = material_find("default/material");
	material_set_float(default_mat, "metallic", 0);

	hand_solid_mat = default_mat;
	hand_ghost_mat = material_copy("app/hand_mat", default_mat);
	material_set_color32(hand_ghost_mat, "color", { 255,255,255,128 });
	material_set_alpha_mode(hand_ghost_mat, material_alpha_blend);

	// Create a PBR floor material
	tex2d_t tex_color = tex2d_create_file("../../Examples/Assets/test.png");
	tex2d_t tex_norm  = tex2d_create_file("../../Examples/Assets/test_normal.png");
	floor_mat = material_create("app/material_floor", shader_find("default/shader_pbr"));
	material_set_texture(floor_mat, "diffuse", tex_color);
	material_set_texture(floor_mat, "normal",  tex_norm);
	material_set_float  (floor_mat, "tex_scale", 6);
	material_set_float  (floor_mat, "roughness", 1.0f);
	material_set_float  (floor_mat, "metallic", 0.5f);
	material_set_queue_offset(floor_mat, 1);
	if (tex_color != nullptr) tex2d_release(tex_color);
	if (tex_norm  != nullptr) tex2d_release(tex_norm);

	// Procedurally create a cube model
	mesh_t mesh_cube = mesh_gen_cube("app/mesh_cube", { 1,1,1 });
	floor_model = model_create_mesh("app/model_cube", mesh_cube, floor_mat);
	mesh_release(mesh_cube);

	// Load a gltf model
	object_model = model_create_file("../../Examples/Assets/DamagedHelmet.gltf");

	// Set hand materials
	button_active_mat = material_copy("app/button_active", default_mat);
	material_set_alpha_mode(button_active_mat, material_alpha_blend);
	button_idle_mat   = material_copy("app/button_idle",   button_active_mat);
	material_set_color(button_active_mat, "color", { 1.f, 0.6f, 0.6f, 0.8f });
	material_set_color(button_idle_mat,   "color", { 0.6f, 1.f, 0.6f, 0.8f });
}

void release_assets() {
	// Release everything
	for (size_t i = 0; i < scene_objects.size(); i++)
		solid_release(scene_objects[i]);

	model_release(object_model);
	model_release(floor_model);

	material_release(floor_mat);
	material_release(hand_ghost_mat);
	material_release(hand_solid_mat);
	material_release(default_mat);
	material_release(button_active_mat);
	material_release(button_idle_mat);
}