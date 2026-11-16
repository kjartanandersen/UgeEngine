#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

// Binding 7. The matrix has no translation, so the sky stays around the camera.
layout(std140, binding = 7) uniform SkyboxData
{
	mat4 u_ViewProjection;
	float u_Intensity;
};

layout(location = 0) out vec3 v_LocalPosition;

void main()
{
	v_LocalPosition = a_Position;

	vec4 position = u_ViewProjection * vec4(a_Position, 1.0);

	// z = w puts the sky on the far plane, which needs GL_LEQUAL to pass.
	gl_Position = position.xyww;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out int entityID;

layout(location = 0) in vec3 v_LocalPosition;

layout(binding = 0) uniform samplerCube u_EnvironmentMap;

layout(std140, binding = 7) uniform SkyboxData
{
	mat4 u_ViewProjection;
	float u_Intensity;
};

void main()
{
	// Linear; the resolve pass tonemaps it.
	fragColor = vec4(texture(u_EnvironmentMap, normalize(v_LocalPosition)).rgb * u_Intensity, 1.0);

	// -1: the sky can't be picked.
	entityID = -1;
}
