#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

layout(std140, binding = 6) uniform CubeRenderData
{
	mat4 u_ViewProjection;
	float u_Roughness;
};

layout(location = 0) out vec3 v_LocalPosition;

void main()
{
	v_LocalPosition = a_Position;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 v_LocalPosition;

layout(binding = 0) uniform samplerCube u_EnvironmentMap;

layout(std140, binding = 6) uniform CubeRenderData
{
	mat4 u_ViewProjection;
	float u_Roughness;
};

const float k_pi = 3.14159265359;

// The first half of Karis' split-sum approximation: the environment convolved against the GGX
// lobe for a fixed roughness. Each mip level of the target holds one roughness, so a rough
// surface reads a blurrier level - which is why this cubemap must have a mip chain.
//
// The view direction is assumed equal to the normal. That is what makes a single prefiltered
// map usable from every angle, and it is also why grazing reflections lose their stretch.

float DistributionGGX(vec3 normal, vec3 halfway, float roughness)
{
	float a = roughness * roughness;
	float aSquared = a * a;
	float normalDotHalfway = max(dot(normal, halfway), 0.0);
	float denominator = normalDotHalfway * normalDotHalfway * (aSquared - 1.0) + 1.0;
	return aSquared / (k_pi * denominator * denominator);
}

// Van der Corput radical inverse - bit-reverses the index to give a low-discrepancy sequence.
float RadicalInverseVdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint count)
{
	return vec2(float(i) / float(count), RadicalInverseVdC(i));
}

// Biases samples towards the GGX lobe, so the samples that matter most are the ones taken.
vec3 ImportanceSampleGGX(vec2 xi, vec3 normal, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.0 * k_pi * xi.x;
	float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	vec3 halfway = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, normal));
	vec3 bitangent = cross(normal, tangent);

	return normalize(tangent * halfway.x + bitangent * halfway.y + normal * halfway.z);
}

const uint k_sampleCount = 1024u;

void main()
{
	vec3 normal = normalize(v_LocalPosition);
	vec3 view = normal;

	vec3 prefilteredColor = vec3(0.0);
	float totalWeight = 0.0;

	for (uint i = 0u; i < k_sampleCount; ++i)
	{
		vec2 xi = Hammersley(i, k_sampleCount);
		vec3 halfway = ImportanceSampleGGX(xi, normal, u_Roughness);
		vec3 light = normalize(2.0 * dot(view, halfway) * halfway - view);

		float normalDotLight = max(dot(normal, light), 0.0);
		if (normalDotLight <= 0.0)
		{
			continue;
		}

		// Sampling a mip chosen by solid angle rather than level 0. Without this a smooth
		// surface reflecting a small bright source produces a field of sparkling dots, because
		// 1024 samples cannot resolve a sun that occupies a handful of texels.
		float distribution = DistributionGGX(normal, halfway, u_Roughness);
		float normalDotHalfway = max(dot(normal, halfway), 0.0);
		float viewDotHalfway = max(dot(view, halfway), 0.0);
		float pdf = distribution * normalDotHalfway / (4.0 * viewDotHalfway) + 0.0001;

		float resolution = float(textureSize(u_EnvironmentMap, 0).x);
		float texelSolidAngle = 4.0 * k_pi / (6.0 * resolution * resolution);
		float sampleSolidAngle = 1.0 / (float(k_sampleCount) * pdf + 0.0001);

		float mipLevel = u_Roughness == 0.0
			? 0.0
			: 0.5 * log2(sampleSolidAngle / texelSolidAngle);

		prefilteredColor += textureLod(u_EnvironmentMap, light, mipLevel).rgb * normalDotLight;
		totalWeight += normalDotLight;
	}

	fragColor = vec4(prefilteredColor / max(totalWeight, 0.001), 1.0);
}
