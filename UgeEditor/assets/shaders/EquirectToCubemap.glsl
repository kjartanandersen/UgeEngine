#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

// Binding 6, shared by the environment build shaders.
layout(std140, binding = 6) uniform CubeRenderData
{
	mat4 u_ViewProjection;
	float u_Roughness;
};

layout(location = 0) out vec3 v_LocalPosition;

void main()
{
	// The cube is drawn from its centre, so a position is also a direction.
	v_LocalPosition = a_Position;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 v_LocalPosition;

layout(binding = 0) uniform sampler2D u_EquirectangularMap;

const vec2 k_invAtan = vec2(0.1591, 0.3183); // 1/(2pi), 1/pi

// A direction's coordinates in an equirectangular image.
vec2 SampleSphericalMap(vec3 direction)
{
	vec2 uv = vec2(atan(direction.z, direction.x), asin(direction.y));
	uv *= k_invAtan;
	uv += 0.5;
	return uv;
}

void main()
{
	vec2 uv = SampleSphericalMap(normalize(v_LocalPosition));

	// .hdr images are linear, so there is no decode.
	fragColor = vec4(texture(u_EquirectangularMap, uv).rgb, 1.0);
}
