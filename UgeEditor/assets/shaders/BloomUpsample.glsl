#type vertex
#version 450 core

// Same fullscreen triangle as Tonemap.glsl; see the comment there for why it overshoots.
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 0) out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;
	gl_Position = vec4(a_Position, 0.0, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 v_TexCoord;

layout(binding = 0) uniform sampler2D u_Source;

layout(std140, binding = 8) uniform BloomData
{
	vec2 u_TexelSize;
	float u_Threshold;
	float u_Knee;
	float u_FilterRadius;
	int u_IsFirstPass;
};

// A 3x3 tent filter, blended additively onto the level below. Widening the kernel by a fixed
// radius in texture space at each level is what turns a stack of downsamples into a smooth,
// wide glow rather than a stack of visible boxes - the radius is in the target's units, so
// the same number spreads further at coarse levels.
//
// The caller sets additive blending; this shader only produces the contribution to add.
void main()
{
	float x = u_FilterRadius * u_TexelSize.x;
	float y = u_FilterRadius * u_TexelSize.y;

	vec3 a = texture(u_Source, v_TexCoord + vec2(-x,  y)).rgb;
	vec3 b = texture(u_Source, v_TexCoord + vec2(0.0, y)).rgb;
	vec3 c = texture(u_Source, v_TexCoord + vec2( x,  y)).rgb;

	vec3 d = texture(u_Source, v_TexCoord + vec2(-x, 0.0)).rgb;
	vec3 e = texture(u_Source, v_TexCoord).rgb;
	vec3 f = texture(u_Source, v_TexCoord + vec2( x, 0.0)).rgb;

	vec3 g = texture(u_Source, v_TexCoord + vec2(-x, -y)).rgb;
	vec3 h = texture(u_Source, v_TexCoord + vec2(0.0, -y)).rgb;
	vec3 i = texture(u_Source, v_TexCoord + vec2( x, -y)).rgb;

	// 1 2 1 / 2 4 2 / 1 2 1, over 16.
	vec3 result = e * 4.0;
	result += (b + d + f + h) * 2.0;
	result += (a + c + g + i);
	result *= 1.0 / 16.0;

	fragColor = vec4(result, 1.0);
}
