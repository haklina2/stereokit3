#include "sk_ui.h"

#include <DirectXMath.h>
using namespace DirectX;
#include <vector>
using namespace std;

struct layer_t {
	XMMATRIX transform;
	vec3     offset;
	float    line_height;
};

vector<layer_t> skui_layers;
mesh_t          skui_box;
material_t      skui_mat;
font_t          skui_font;
text_style_t    skui_font_style;
material_t      skui_font_mat;
transform_t     skui_tr;

float skui_padding = 10*mm2m;
float skui_gutter  = 20*mm2m;
float skui_depth   = 20*mm2m;
float skui_fontsize= 20 * mm2m;

void sk_ui_init() {
	skui_box = mesh_gen_cube("sk_ui/box", vec3_one);
	skui_mat = material_find("default/material");

	skui_font_mat   = material_create("app/font_segoe", shader_find("default/shader_font"));
	skui_font       = font_create("C:/Windows/Fonts/segoeui.ttf");
	skui_font_style = text_make_style(skui_font, skui_fontsize, skui_font_mat, text_align_x_left | text_align_y_top);
}

void sk_ui_begin_frame() {
}

void sk_ui_push_pose(pose_t pose) {
	skui_layers.push_back(layer_t{
		XMMatrixAffineTransformation(
			DirectX::g_XMOne, DirectX::g_XMZero,
			XMLoadFloat4((XMFLOAT4 *)& pose.orientation),
			XMLoadFloat3((XMFLOAT3 *)& pose.position)),
		vec3{skui_padding, skui_padding}, 0,
	});
}

void sk_ui_pop_pose() {
	skui_layers.pop_back();
}

void sk_ui_box(vec3 start, vec3 size) {
	vec3 pos = start + (vec3{ size.x, -size.y, size.z } / 2);
	XMMATRIX mat = XMMatrixAffineTransformation(
			XMLoadFloat3((XMFLOAT3 *)& size), DirectX::g_XMZero,
			DirectX::g_XMIdentityR3,
			XMLoadFloat3((XMFLOAT3 *)&pos));
	mat *= skui_layers.back().transform;

	render_add_mesh_mx(skui_box, skui_mat, mat);
}

void sk_ui_text(vec3 start, const char *text) {
	skui_tr._dirty     = false;
	skui_tr._transform = skui_layers.back().transform;

	text_add_at(skui_font_style, skui_tr, text, start.x, start.y, start.z);
}

void sk_ui_reserve_box(vec2 size) {
	skui_layers.back().offset += vec3{ size.x + skui_gutter, 0, 0 };
	if (skui_layers.back().line_height < size.y)
		skui_layers.back().line_height = size.y;
}
void sk_ui_nextline() {
	skui_layers.back().offset.x = skui_padding;
	skui_layers.back().offset.y -= skui_layers.back().line_height + skui_gutter;
}
void sk_ui_space(float space) {
	if (skui_layers.back().offset.x == skui_padding)
		skui_layers.back().offset.y -= space;
	else
		skui_layers.back().offset.x += space;
}

void sk_ui_button(const char *text) {
	vec3 offset = skui_layers.back().offset;
	vec2 size   = text_size(skui_font_style, text);
	size += vec2{ skui_padding, skui_padding }*2;

	sk_ui_reserve_box(size);
	sk_ui_box(offset, vec3{ size.x, size.y, skui_depth });
	sk_ui_text(offset + vec3{ skui_padding, -skui_padding, (skui_depth + 2*mm2m) }, text);
}