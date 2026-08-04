#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;

// Binding 7, its own block rather than the mesh pass camera at binding 3: the skybox needs
// the view matrix with translation stripped, so that moving the camera turns the sky but
// never travels through it.
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

	// Forces depth to the far plane by making z equal w, so the sky sits behind everything.
	// Drawn last with GL_LEQUAL rather than GL_LESS, since z/w lands exactly on 1.0 and a
	// strict test would reject every fragment.
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
	// Linear and unbounded, like everything else written into the scene target; the resolve
	// pass tonemaps it. @see Uge::PostProcess
	fragColor = vec4(texture(u_EnvironmentMap, normalize(v_LocalPosition)).rgb * u_Intensity, 1.0);

	// The sky is not pickable, and -1 is what the editor clears the ID attachment to. Writing
	// it explicitly matters because the attachment is never cleared again after the scene is
	// drawn, so leaving this out would let the sky keep whatever ID was there before.
	entityID = -1;
}
