#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

// Binding 6, used by all three environment build passes. They run once at import, never
// during a frame, so they share a binding point with nothing the scene pass uses.
layout(std140, binding = 6) uniform CubeRenderData
{
	mat4 u_ViewProjection;
	float u_Roughness;
};

layout(location = 0) out vec3 v_LocalPosition;

void main()
{
	// The cube is rendered from its own centre, so the vertex position doubles as the
	// direction the fragment is looking in.
	v_LocalPosition = a_Position;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 v_LocalPosition;

layout(binding = 0) uniform sampler2D u_EquirectangularMap;

const vec2 k_invAtan = vec2(0.1591, 0.3183); // 1/(2pi), 1/pi

// Maps a direction onto an equirectangular (latitude-longitude) image. The horizontal angle
// wraps a full turn, the vertical covers pole to pole, which is why the two constants differ
// by a factor of two.
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

	// The source is already linear - Radiance .hdr stores radiance, not gamma-encoded colour -
	// so there is no decode here, unlike the sRGB textures Model.glsl samples.
	fragColor = vec4(texture(u_EquirectangularMap, uv).rgb, 1.0);
}
