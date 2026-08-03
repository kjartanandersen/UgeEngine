#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

layout(std140, binding = 1) uniform ModelData
{
	mat4 u_Model;
	int u_EntityData;
};

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out flat int v_EntityID;

void main()
{
	vec4 worldPosition = u_Model * vec4(a_Position, 1.0);
	v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	v_TexCoord = a_TexCoord;
	v_EntityID = u_EntityData;

	gl_Position = u_ViewProjection * worldPosition;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out int entityID;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in flat int v_EntityID;

layout(binding = 0) uniform sampler2D u_AlbedoMap;

// Mirrors Uge::MaterialUniformData. Keep the two in step: a layout mismatch shows up as
// wrong values, not as an error.
layout(std140, binding = 2) uniform MaterialData
{
	vec4 u_AlbedoColor;
	float u_Roughness;
	float u_Metallic;
	float u_EmissiveStrength;
	float u_AlphaCutoff;
	int u_MapFlags;
	int u_BlendMode;
};

// Uge::MaterialMapFlags
const int MAP_ALBEDO = 1 << 0;

// Uge::AlphaMode
const int ALPHA_MODE_MASK = 1;

// Shading happens in linear space, but colour textures are sRGB encoded and the render
// target is read back as sRGB, so inputs are decoded on the way in and the result encoded
// on the way out. Skipping both cancels out for textured surfaces and is exactly why an
// untextured linear baseColorFactor came out too dark.
//
// The conversion is done here rather than through a GL_SRGB8_ALPHA8 texture format because
// EditorAssetManager::GetOrImportAsset shares one texture asset between every material that
// references the file - the colour space belongs to the slot it is bound to, not the file.

vec3 SrgbToLinear(vec3 color)
{
	vec3 low = color / 12.92;
	vec3 high = pow((color + 0.055) / 1.055, vec3(2.4));
	return mix(low, high, step(vec3(0.04045), color));
}

vec3 LinearToSrgb(vec3 color)
{
	vec3 low = color * 12.92;
	vec3 high = 1.055 * pow(color, vec3(1.0 / 2.4)) - 0.055;
	return mix(low, high, step(vec3(0.0031308), color));
}

void main()
{
	// A material with no albedo map is not untextured, it is a solid colour - the car body
	// paint, the tyre rubber and the glass all arrive that way. Factors are already linear;
	// only the texture needs decoding.
	vec4 albedo = u_AlbedoColor;
	if ((u_MapFlags & MAP_ALBEDO) != 0)
	{
		vec4 albedoSample = texture(u_AlbedoMap, v_TexCoord);

		// Alpha is never gamma encoded, so it passes through untouched.
		albedo *= vec4(SrgbToLinear(albedoSample.rgb), albedoSample.a);
	}

	if (u_BlendMode == ALPHA_MODE_MASK && albedo.a < u_AlphaCutoff)
	{
		discard;
	}

	vec3 normal = normalize(v_Normal);
	vec3 lightDirection = normalize(vec3(0.4, 0.8, 0.2));
	float diffuse = max(dot(normal, lightDirection), 0.1);

	vec3 shaded = albedo.rgb * diffuse;

	fragColor = vec4(LinearToSrgb(shaded), albedo.a);
	entityID = v_EntityID;
}
