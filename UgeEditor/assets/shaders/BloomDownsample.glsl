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

// Mirrors Uge::BloomUniformData. Keep the two in step.
layout(std140, binding = 8) uniform BloomData
{
	vec2 u_TexelSize;
	float u_Threshold;
	float u_Knee;
	float u_FilterRadius;
	int u_IsFirstPass;
};

// Rejects anything that is not a finite number before it enters the chain. A single bad
// texel does not stay a single bad texel here: it is averaged into its neighbours at every
// level and then spread back out on the way up, so one NaN becomes a large hard-edged
// rectangle. The shading pass clamps its own output, but this pyramid is where such a value
// does the most damage, so it is worth refusing one at the door.
vec3 Sanitize(vec3 color)
{
	bvec3 bad = bvec3(isnan(color.r) || isinf(color.r),
	                  isnan(color.g) || isinf(color.g),
	                  isnan(color.b) || isinf(color.b));
	return mix(color, vec3(0.0), bad);
}

// Karis average: weight each sample by the inverse of its own brightness before averaging.
// Without it a single very bright pixel - a headlight lens, a specular pinpoint - dominates
// the 13 taps and survives every downsample as an isolated flickering dot. This is the
// standard fix for bloom fireflies, and it is only applied on the first pass because after
// that the signal is already averaged.
float KarisWeight(vec3 color)
{
	// Rec. 709 luma, then the reciprocal of "one plus brightness".
	float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
	return 1.0 / (1.0 + luma);
}

// Soft-knee threshold. A hard cutoff makes bloom pop on and off as a surface crosses the
// threshold; the knee ramps contribution in over a range instead.
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

	// The 13-tap pattern from Jimenez's SIGGRAPH 2014 talk: four inner samples forming a box
	// plus a 3x3 grid, combined so the result is a wide, artifact-free downsample. A plain
	// bilinear halving aliases badly once it has been repeated six times.
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
		// Each group is averaged with its own Karis weight, then the groups are combined.
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

		// The centre group carries half the weight, the corners an eighth each.
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
