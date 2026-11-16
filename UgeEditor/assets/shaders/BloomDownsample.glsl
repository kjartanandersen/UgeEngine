#type vertex
#version 450 core

// The fullscreen triangle, as in Tonemap.glsl.
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

// Matches BloomUniformData in Bloom.cpp.
layout(std140, binding = 8) uniform BloomData
{
	vec2 u_TexelSize;
	float u_Threshold;
	float u_Knee;
	float u_FilterRadius;
	int u_IsFirstPass;
};

// Replaces NaN and infinity, which the chain would spread into a large black rectangle.
vec3 Sanitize(vec3 color)
{
	bvec3 bad = bvec3(isnan(color.r) || isinf(color.r),
	                  isnan(color.g) || isinf(color.g),
	                  isnan(color.b) || isinf(color.b));
	return mix(color, vec3(0.0), bad);
}

// Karis average: each sample weighted by 1 / (1 + luma), so one very bright pixel doesn't
// become a flickering dot. First pass only.
float KarisWeight(vec3 color)
{
	// Rec. 709 luma.
	float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
	return 1.0 / (1.0 + luma);
}

// Threshold with a soft knee, so bloom fades in.
vec3 ApplyThreshold(vec3 color)
{
	float brightness = max(color.r, max(color.g, color.b));

	float soft = brightness - u_Threshold + u_Knee;
	soft = clamp(soft, 0.0, 2.0 * u_Knee);
	soft = soft * soft / (4.0 * u_Knee + 0.0001);

	float contribution = max(soft, brightness - u_Threshold) / max(brightness, 0.0001);
	return color * contribution;
}

void main()
{
	vec2 texel = u_TexelSize;

	// The 13-tap downsample from Jimenez's SIGGRAPH 2014 talk: an inner box of four and a
	// 3x3 grid.
	vec3 a = texture(u_Source, v_TexCoord + texel * vec2(-2.0,  2.0)).rgb;
	vec3 b = texture(u_Source, v_TexCoord + texel * vec2( 0.0,  2.0)).rgb;
	vec3 c = texture(u_Source, v_TexCoord + texel * vec2( 2.0,  2.0)).rgb;

	vec3 d = texture(u_Source, v_TexCoord + texel * vec2(-2.0,  0.0)).rgb;
	vec3 e = texture(u_Source, v_TexCoord).rgb;
	vec3 f = texture(u_Source, v_TexCoord + texel * vec2( 2.0,  0.0)).rgb;

	vec3 g = texture(u_Source, v_TexCoord + texel * vec2(-2.0, -2.0)).rgb;
	vec3 h = texture(u_Source, v_TexCoord + texel * vec2( 0.0, -2.0)).rgb;
	vec3 i = texture(u_Source, v_TexCoord + texel * vec2( 2.0, -2.0)).rgb;

	vec3 j = texture(u_Source, v_TexCoord + texel * vec2(-1.0,  1.0)).rgb;
	vec3 k = texture(u_Source, v_TexCoord + texel * vec2( 1.0,  1.0)).rgb;
	vec3 l = texture(u_Source, v_TexCoord + texel * vec2(-1.0, -1.0)).rgb;
	vec3 m = texture(u_Source, v_TexCoord + texel * vec2( 1.0, -1.0)).rgb;

	vec3 result;
	if (u_IsFirstPass != 0)
	{
		// Each group gets its own Karis weight.
		vec3 group0 = (a + b + d + e) * 0.25;
		vec3 group1 = (b + c + e + f) * 0.25;
		vec3 group2 = (d + e + g + h) * 0.25;
		vec3 group3 = (e + f + h + i) * 0.25;
		vec3 group4 = (j + k + l + m) * 0.25;

		float w0 = KarisWeight(group0);
		float w1 = KarisWeight(group1);
		float w2 = KarisWeight(group2);
		float w3 = KarisWeight(group3);
		float w4 = KarisWeight(group4);

		// The centre group has half the weight, each corner an eighth.
		float total = w0 * 0.125 + w1 * 0.125 + w2 * 0.125 + w3 * 0.125 + w4 * 0.5;
		result = (group0 * w0 * 0.125 + group1 * w1 * 0.125 + group2 * w2 * 0.125
				+ group3 * w3 * 0.125 + group4 * w4 * 0.5) / max(total, 0.0001);

		result = ApplyThreshold(Sanitize(result));
	}
	else
	{
		result  = e * 0.125;
		result += (a + c + g + i) * 0.03125;
		result += (b + d + f + h) * 0.0625;
		result += (j + k + l + m) * 0.125;
	}

	fragColor = vec4(result, 1.0);
}
