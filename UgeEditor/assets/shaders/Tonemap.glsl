#type vertex
#version 450 core

// A fullscreen triangle. Its corners reach 3, so the screen is the part from -1 to 1.
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

// The linear HDR scene.
layout(binding = 0) uniform sampler2D u_SceneColor;

// The bloom result, at half the scene's resolution.
layout(binding = 1) uniform sampler2D u_BloomColor;

// Matches TonemapUniformData in PostProcess.cpp.
layout(std140, binding = 5) uniform TonemapData
{
	float u_Exposure;
	int u_TonemapMode;
	float u_BloomIntensity;
};

// Uge::TonemapMode
const int TONEMAP_NONE = 0;
const int TONEMAP_REINHARD = 1;
const int TONEMAP_ACES = 2;

vec3 LinearToSrgb(vec3 color)
{
	vec3 low = color * 12.92;
	vec3 high = 1.055 * pow(color, vec3(1.0 / 2.4)) - 0.055;
	return mix(low, high, step(vec3(0.0031308), color));
}

// Narkowicz's fit to the ACES filmic curve.
vec3 TonemapAces(vec3 color)
{
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

void main()
{
	vec3 color = texture(u_SceneColor, v_TexCoord).rgb;

	// Added before exposure and the curve, so bloom is tonemapped too.
	color += texture(u_BloomColor, v_TexCoord).rgb * u_BloomIntensity;

	color *= u_Exposure;

	switch (u_TonemapMode)
	{
		case TONEMAP_REINHARD:
			color = color / (color + 1.0);
			break;
		case TONEMAP_ACES:
			color = TonemapAces(color);
			break;
		default:
			// TONEMAP_NONE: no curve, but the sRGB encode still runs. The RGBA8 target clips
			// values above 1.
			break;
	}

	fragColor = vec4(LinearToSrgb(color), 1.0);
}
