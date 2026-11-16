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

layout(location = 0) out vec2 fragColor;

layout(location = 0) in vec2 v_TexCoord;

const float k_pi = 3.14159265359;

// The split sum's second half: a scale and bias on F0, by view angle (x) and roughness (y).

float RadicalInverseVdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint i, uint count)
{
	return vec2(float(i) / float(count), RadicalInverseVdC(i));
}

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

// k = roughness^2 / 2, the image-based form. Model.glsl uses (roughness + 1)^2 / 8.
float GeometrySchlickGGX(float normalDotVector, float roughness)
{
	float a = roughness;
	float k = (a * a) / 2.0;
	return normalDotVector / (normalDotVector * (1.0 - k) + k);
}

float GeometrySmith(vec3 normal, vec3 view, vec3 light, float roughness)
{
	return GeometrySchlickGGX(max(dot(normal, view), 0.0), roughness)
		 * GeometrySchlickGGX(max(dot(normal, light), 0.0), roughness);
}

const uint k_sampleCount = 1024u;

vec2 IntegrateBRDF(float normalDotView, float roughness)
{
	vec3 view = vec3(sqrt(1.0 - normalDotView * normalDotView), 0.0, normalDotView);
	vec3 normal = vec3(0.0, 0.0, 1.0);

	float scale = 0.0;
	float bias = 0.0;

	for (uint i = 0u; i < k_sampleCount; ++i)
	{
		vec2 xi = Hammersley(i, k_sampleCount);
		vec3 halfway = ImportanceSampleGGX(xi, normal, roughness);
		vec3 light = normalize(2.0 * dot(view, halfway) * halfway - view);

		float normalDotLight = max(light.z, 0.0);
		if (normalDotLight <= 0.0)
		{
			continue;
		}

		float normalDotHalfway = max(halfway.z, 0.0);
		float viewDotHalfway = max(dot(view, halfway), 0.0);

		float geometry = GeometrySmith(normal, view, light, roughness);
		float visibility = (geometry * viewDotHalfway) / (normalDotHalfway * normalDotView);
		float fresnelTerm = pow(1.0 - viewDotHalfway, 5.0);

		scale += (1.0 - fresnelTerm) * visibility;
		bias += fresnelTerm * visibility;
	}

	return vec2(scale, bias) / float(k_sampleCount);
}

void main()
{
	// x is cos(view angle), y is roughness. Both stay above zero, as a zero view angle
	// divides by zero.
	fragColor = IntegrateBRDF(max(v_TexCoord.x, 0.001), max(v_TexCoord.y, 0.001));
}
