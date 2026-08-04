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

// Integrates incoming radiance over the hemisphere around the surface normal, weighted by
// cosine. The result is what a perfectly rough (Lambertian) surface facing this direction
// would reflect, which is the diffuse half of image-based lighting.
//
// Done once at import because the integral depends only on the environment. At render time
// this collapses to a single texture fetch - which is the whole point.
void main()
{
	vec3 normal = normalize(v_LocalPosition);

	// An orthonormal basis around the normal, so the samples below can be generated in a
	// convenient space and rotated into world space.
	vec3 up = abs(normal.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);
	vec3 right = normalize(cross(up, normal));
	up = cross(normal, right);

	vec3 irradiance = vec3(0.0);
	float sampleCount = 0.0;

	// Uniform angular steps rather than importance sampling: the integrand is smooth and the
	// target is only 32x32, so a regular grid converges perfectly well and stays readable.
	const float k_sampleDelta = 0.025;

	for (float phi = 0.0; phi < 2.0 * k_pi; phi += k_sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * k_pi; theta += k_sampleDelta)
		{
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 worldSample = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

			// cos(theta) is the Lambert term; sin(theta) is the solid angle of the sample,
			// which shrinks towards the pole and stops it being over-weighted.
			irradiance += texture(u_EnvironmentMap, worldSample).rgb * cos(theta) * sin(theta);
			sampleCount += 1.0;
		}
	}

	irradiance = k_pi * irradiance * (1.0 / sampleCount);

	fragColor = vec4(irradiance, 1.0);
}
