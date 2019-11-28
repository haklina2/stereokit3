const char* sk_shader_builtin_lines = R"_(
// [name] sk/lines
cbuffer GlobalBuffer : register(b0) {
	float4x4 sk_view[2];
	float4x4 sk_proj[2];
	float4x4 sk_viewproj[2];
	float4   sk_light;
	float4   sk_light_color;
	float4   sk_camera_pos[2];
	float4   sk_camera_dir[2];
	float4   sk_fingertip[2];
	float    sk_time;
};
struct Inst {
	float4x4 world;
	float4   color;
};
cbuffer TransformBuffer : register(b1) {
	Inst sk_inst[1];
};
cbuffer ParamBuffer : register(b2) {
	// [ param ] vector color {1, 1, 1, 1}
	float4 _color;
};
struct vsIn {
	float4 pos  : SV_POSITION;
	float4 col  : COLOR;
	float3 norm : NORMAL;
	float2 uv   : TEXCOORD0;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float4 color : COLOR0;
	float2 uv    : TEXCOORD0;
};

// [texture] diffuse white
Texture2D tex : register(t0);
SamplerState tex_sampler;

psIn vs(vsIn input, uint id : SV_InstanceID, uint view_id : SV_RenderTargetArrayIndex) {
	psIn output;
	float4 view  = mul(float4(input.pos.xyz, 1), sk_viewproj[view_id]);
	float3 norm  = mul(float4(input.norm,0), sk_viewproj[view_id]).xyz;
	view.xy += float2(norm.y, -norm.x);
	output.pos   = view;// mul(view, sk_proj[view_id]);
	output.uv    = input.uv;
	output.color = input.col;
	return output;
}
float4 ps(psIn input) : SV_TARGET {
	float4 col = tex.Sample(tex_sampler, input.uv);
	//clip(col.a-0.01);

	col = col * input.color;

	return col; 
})_";