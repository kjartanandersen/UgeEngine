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

const float k_pi = 3.14159265359;

// Integrates the light over the hemisphere around the normal, weighted by cosine: the
// diffuse light for a Lambertian surface.
void main()
{
	vec3 normal = normalize(v_LocalPosition);

	// A basis around the normal, to turn the samples into world space.
	vec3 up = abs(normal.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);
	vec3 right = normalize(cross(up, normal));
	up = cross(normal, right);

	vec3 irradiance = vec3(0.0);
	float sampleCount = 0.0;

	// Uniform steps; the integrand is smooth enough not to need importance sampling.
	const float k_sampleDelta = 0.025;

	for (float phi = 0.0; phi < 2.0 * k_pi; phi += k_sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * k_pi; theta += k_sampleDelta)
		{
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 worldSample = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

			// cos(theta) is the Lambert term; sin(theta) scales for the sample's solid angle.
			irradiance += texture(u_EnvironmentMap, worldSample).rgb * cos(theta) * sin(theta);
			sampleCount += 1.0;
		}
	}

	irradiance = k_pi * irradiance * (1.0 / sampleCount);

	fragColor = vec4(irradiance, 1.0);
}
