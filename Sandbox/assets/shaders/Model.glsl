#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in int a_HasDiffuseMap;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

layout(std140, binding = 1) uniform ModelData
{
	mat4 u_Model;
	ivec4 u_EntityData;
};

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out flat int v_HasDiffuseMap;
layout(location = 3) out flat int v_EntityID;

void main()
{
	vec4 worldPosition = u_Model * vec4(a_Position, 1.0);
	v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	v_TexCoord = a_TexCoord;
	v_HasDiffuseMap = a_HasDiffuseMap;
	v_EntityID = u_EntityData.x;

	gl_Position = u_ViewProjection * worldPosition;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out int entityID;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in flat int v_HasDiffuseMap;
layout(location = 3) in flat int v_EntityID;

layout(binding = 2) uniform sampler2D u_DiffuseMap;

void main()
{
	vec3 albedo = vec3(0.85, 0.85, 0.85);
	if (v_HasDiffuseMap == 1)
	{
		albedo = texture(u_DiffuseMap, v_TexCoord).rgb;
	}

	vec3 normal = normalize(v_Normal);
	vec3 lightDirection = normalize(vec3(0.4, 0.8, 0.2));
	float diffuse = max(dot(normal, lightDirection), 0.1);

	fragColor = vec4(albedo * diffuse, 1.0);
	entityID = v_EntityID;
}
