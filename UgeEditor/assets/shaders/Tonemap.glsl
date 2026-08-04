#type vertex
#version 450 core

// A fullscreen triangle, not a quad: one primitive instead of two, and no seam along the
// diagonal where the two halves of a quad meet. The vertex data deliberately overshoots the
// screen - positions reach 3.0 and texture coordinates reach 2.0 - so the visible [-1, 1]
// region is the inscribed portion of a triangle twice its size.
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

// The scene's HDR colour attachment. Holds linear radiance, which is unbounded above -
// an emissive car headlight lands well past 1.0.
layout(binding = 0) uniform sampler2D u_SceneColor;

// The accumulated bloom chain, at half the scene's resolution. @see Uge::Bloom
layout(binding = 1) uniform sampler2D u_BloomColor;

// Mirrors Uge::TonemapUniformData. Keep the two in step: a layout mismatch shows up as
// wrong values, not as an error.
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

// Narkowicz's fit to the ACES filmic curve. Cheap, and it rolls highlights off towards white
// instead of clipping them to a flat colour, which is what stops a bright specular reading as
// a solid saturated blob.
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

	// Added before exposure and the curve, so the glow is tonemapped along with everything
	// else. Adding it afterwards would let it survive the highlight roll-off untouched and
	// read as a flat overlay pasted on top of the image.
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
			// TONEMAP_NONE. Not "no processing" - the encode below still runs. This is the
			// mode that reproduces what the pipeline did before the HDR target existed, when
			// Model.glsl encoded to sRGB itself and the RGBA8 attachment clamped the result.
			// Values above 1.0 are clipped by the RGBA8 target rather than rolled off.
			break;
	}

	fragColor = vec4(LinearToSrgb(color), 1.0);
}
