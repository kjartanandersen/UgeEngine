#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

// Binding 3, as Renderer2D's smaller camera block is at 0.
layout(std140, binding = 3) uniform Camera
{
	mat4 u_ViewProjection;
	vec4 u_CameraPosition;
};

layout(std140, binding = 1) uniform ModelData
{
	mat4 u_Model;
	int u_EntityData;
};

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out flat int v_EntityID;
layout(location = 3) out vec3 v_WorldPosition;
layout(location = 4) out vec3 v_ViewDirection;

void main()
{
	vec4 worldPosition = u_Model * vec4(a_Position, 1.0);
	v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
	v_TexCoord = a_TexCoord;
	v_EntityID = u_EntityData;

	// The fragment stage derives the tangent frame from this.
	v_WorldPosition = worldPosition.xyz;

	// Passed down, as a block declared in both stages gets two names after SPIR-V, which
	// some drivers won't link.
	v_ViewDirection = u_CameraPosition.xyz - worldPosition.xyz;

	gl_Position = u_ViewProjection * worldPosition;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out int entityID;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec2 v_TexCoord;
layout(location = 2) in flat int v_EntityID;
layout(location = 3) in vec3 v_WorldPosition;
layout(location = 4) in vec3 v_ViewDirection;

// Slot order matches OpenGLMaterial::Bind.
layout(binding = 0) uniform sampler2D u_AlbedoMap;
layout(binding = 1) uniform sampler2D u_NormalMap;
layout(binding = 2) uniform sampler2D u_RoughnessMap;
layout(binding = 3) uniform sampler2D u_MetallicMap;
layout(binding = 4) uniform sampler2D u_AmbientOcclusionMap;
layout(binding = 5) uniform sampler2D u_EmissiveMap;

// Image-based lighting, bound once per frame.
layout(binding = 6) uniform samplerCube u_IrradianceMap;
layout(binding = 7) uniform samplerCube u_PrefilterMap;
layout(binding = 8) uniform sampler2D u_BrdfLut;

// Matches Uge::MaterialUniformData.
layout(std140, binding = 2) uniform MaterialData
{
	vec4 u_AlbedoColor;
	vec4 u_EmissiveColor;
	float u_Roughness;
	float u_Metallic;
	float u_EmissiveStrength;
	float u_AlphaCutoff;
	int u_MapFlags;
	int u_BlendMode;
};

// The directional light and the environment, per frame; matches Uge::LightData. Zero
// radiance means there is no directional light.
layout(std140, binding = 4) uniform LightData
{
	vec4 u_LightDirection;
	vec4 u_LightRadiance;
	int u_HasEnvironment;
	int u_EnvironmentMipCount;
	float u_EnvironmentIntensity;
};

// Uge::MaterialMapFlags. A slot whose flag is clear holds the previous material's texture.
const int MAP_ALBEDO = 1 << 0;
const int MAP_NORMAL = 1 << 1;
const int MAP_ROUGHNESS = 1 << 2;
const int MAP_METALLIC = 1 << 3;
const int MAP_AMBIENT_OCCLUSION = 1 << 4;
const int MAP_EMISSIVE = 1 << 5;
const int MAP_PACKED_METALLIC_ROUGHNESS = 1 << 6;


// Uge::AlphaMode
const int ALPHA_MODE_MASK = 1;

// Writes linear light; the resolve pass encodes sRGB. Albedo and emissive textures are
// decoded from sRGB here, not by an sRGB texture format, as one texture can be bound to
// colour and non-colour slots.

vec3 SrgbToLinear(vec3 color)
{
	vec3 low = color / 12.92;
	vec3 high = pow((color + 0.055) / 1.055, vec3(2.4));
	return mix(low, high, step(vec3(0.04045), color));
}

// A tangent frame from screen-space derivatives, as MeshVertex has no tangents.
mat3 CotangentFrame(vec3 normal, vec3 position, vec2 uv)
{
	vec3 positionDx = dFdx(position);
	vec3 positionDy = dFdy(position);
	vec2 uvDx = dFdx(uv);
	vec2 uvDy = dFdy(uv);

	vec3 perpDy = cross(positionDy, normal);
	vec3 perpDx = cross(normal, positionDx);

	vec3 tangent = perpDy * uvDx.x + perpDx * uvDy.x;
	vec3 bitangent = perpDy * uvDx.y + perpDx * uvDy.y;

	// Degenerate UVs give a zero basis; the clamp keeps inversesqrt finite.
	float maxLengthSquared = max(max(dot(tangent, tangent), dot(bitangent, bitangent)), 1e-12);
	float invMax = inversesqrt(maxLengthSquared);

	return mat3(tangent * invMax, bitangent * invMax, normal);
}

const float k_pi = 3.14159265359;

// ---- Cook-Torrance BRDF ----
//
// D * G * F / (4 * NdotL * NdotV): the distribution, geometry and Fresnel terms.

// Trowbridge-Reitz GGX, with roughness squared so it looks linear.
float DistributionGGX(vec3 normal, vec3 halfway, float roughness)
{
	float a = roughness * roughness;
	float aSquared = a * a;
	float normalDotHalfway = max(dot(normal, halfway), 0.0);
	float denominator = normalDotHalfway * normalDotHalfway * (aSquared - 1.0) + 1.0;
	return aSquared / (k_pi * denominator * denominator);
}

// Schlick-GGX with direct lighting's k = (roughness + 1)^2 / 8. BrdfLut.glsl uses
// roughness^2 / 2.
float GeometrySchlickGGX(float normalDotVector, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;
	return normalDotVector / (normalDotVector * (1.0 - k) + k);
}

float GeometrySmith(float normalDotView, float normalDotLight, float roughness)
{
	return GeometrySchlickGGX(normalDotView, roughness)
		 * GeometrySchlickGGX(normalDotLight, roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Fresnel for the ambient term, limited by roughness so rough surfaces get no bright rim.
vec3 FresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
	vec3 clamped = max(vec3(1.0 - roughness), f0);
	return f0 + (clamped - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
	// Without an albedo map the colour factor is used alone. Factors are already linear.
	vec4 albedo = u_AlbedoColor;
	if ((u_MapFlags & MAP_ALBEDO) != 0)
	{
		vec4 albedoSample = texture(u_AlbedoMap, v_TexCoord);

		// Alpha isn't sRGB encoded.
		albedo *= vec4(SrgbToLinear(albedoSample.rgb), albedoSample.a);
	}

	if (u_BlendMode == ALPHA_MODE_MASK && albedo.a < u_AlphaCutoff)
	{
		discard;
	}

	vec3 normal = normalize(v_Normal);

	// A back face of a double-sided material is lit with its normal flipped, before the
	// normal map so the frame flips too.
	if (!gl_FrontFacing)
	{
		normal = -normal;
	}

	if ((u_MapFlags & MAP_NORMAL) != 0)
	{
		vec3 tangentNormal = texture(u_NormalMap, v_TexCoord).xyz * 2.0 - 1.0;
		normal = normalize(CotangentFrame(normal, v_WorldPosition, v_TexCoord) * tangentNormal);
	}

	// glTF packs roughness in green and metalness in blue; otherwise they are two maps.
	float roughness = u_Roughness;
	float metallic = u_Metallic;
	if ((u_MapFlags & MAP_PACKED_METALLIC_ROUGHNESS) != 0)
	{
		vec4 metallicRoughnessSample = texture(u_RoughnessMap, v_TexCoord);
		roughness *= metallicRoughnessSample.g;
		metallic *= metallicRoughnessSample.b;
	}
	else
	{
		if ((u_MapFlags & MAP_ROUGHNESS) != 0)
		{
			roughness *= texture(u_RoughnessMap, v_TexCoord).r;
		}
		if ((u_MapFlags & MAP_METALLIC) != 0)
		{
			metallic *= texture(u_MetallicMap, v_TexCoord).r;
		}
	}

	// Roughness 0 makes GGX 0/0, so it has a floor.
	roughness = clamp(roughness, 0.025, 1.0);

	float ambientOcclusion = 1.0;
	if ((u_MapFlags & MAP_AMBIENT_OCCLUSION) != 0)
	{
		ambientOcclusion = texture(u_AmbientOcclusionMap, v_TexCoord).r;
	}

	// Normalized, and pointing towards the light.
	vec3 lightDirection = u_LightDirection.xyz;
	vec3 lightRadiance = u_LightRadiance.rgb;

	vec3 viewDirection = normalize(v_ViewDirection);
	vec3 halfway = normalize(lightDirection + viewDirection);

	float normalDotView = max(dot(normal, viewDirection), 0.0);
	float normalDotLight = max(dot(normal, lightDirection), 0.0);

	// Reflectance at normal incidence: 4% for dielectrics, the albedo for metals.
	vec3 f0 = mix(vec3(0.04), albedo.rgb, metallic);

	// ---- Direct lighting: Cook-Torrance ----

	float distribution = DistributionGGX(normal, halfway, roughness);
	float geometry = GeometrySmith(normalDotView, normalDotLight, roughness);
	vec3 fresnel = FresnelSchlick(max(dot(halfway, viewDirection), 0.0), f0);

	// The epsilon guards grazing angles, where both dot products go to zero.
	vec3 specular = (distribution * geometry * fresnel)
		/ (4.0 * normalDotView * normalDotLight + 0.0001);

	// A directional light has no size, so a smooth surface's highlight is unbounded. The
	// clamp keeps it from swamping bloom.
	const float k_maxDirectSpecular = 50.0;
	specular = min(specular, vec3(k_maxDirectSpecular));

	// Light reflected specularly isn't diffused as well. Metals have no diffuse.
	vec3 diffuseWeight = (vec3(1.0) - fresnel) * (1.0 - metallic);

	vec3 shaded = (diffuseWeight * albedo.rgb / k_pi + specular) * lightRadiance * normalDotLight;

	// ---- Ambient: image-based lighting ----

	if (u_HasEnvironment != 0)
	{
		// Diffuse: the irradiance map is already convolved.
		vec3 irradiance = texture(u_IrradianceMap, normal).rgb;
		vec3 diffuseIBL = irradiance * albedo.rgb;

		// Specular, by the split sum: the prefiltered map's mip for this roughness, times the
		// lookup table's scale and bias on F0.
		vec3 reflected = reflect(-viewDirection, normal);
		float mipLevel = roughness * float(u_EnvironmentMipCount - 1);
		vec3 prefiltered = textureLod(u_PrefilterMap, reflected, mipLevel).rgb;

		vec2 environmentBrdf = texture(u_BrdfLut, vec2(normalDotView, roughness)).rg;
		vec3 specularIBL = prefiltered * (f0 * environmentBrdf.x + environmentBrdf.y);

		// Fresnel from the view angle, as there is no half vector.
		vec3 ambientFresnel = FresnelSchlickRoughness(normalDotView, f0, roughness);
		vec3 ambientDiffuseWeight = (vec3(1.0) - ambientFresnel) * (1.0 - metallic);

		shaded += (ambientDiffuseWeight * diffuseIBL + specularIBL)
			* ambientOcclusion * u_EnvironmentIntensity;
	}
	else
	{
		// No environment: a flat ambient term scaled by the light, with a floor for scenes
		// without one.
		shaded += albedo.rgb * max(0.08 * lightRadiance, vec3(0.03)) * ambientOcclusion;
	}

	if ((u_MapFlags & MAP_EMISSIVE) != 0)
	{
		vec3 emissiveSample = SrgbToLinear(texture(u_EmissiveMap, v_TexCoord).rgb);
		shaded += emissiveSample * u_EmissiveColor.rgb * u_EmissiveStrength;
	}
	else
	{
		shaded += u_EmissiveColor.rgb * u_EmissiveStrength;
	}

	// Clamped below RGBA16F's maximum of 65504, as an infinity becomes NaN in bloom.
	const float k_maxRadiance = 1000.0;
	shaded = min(shaded, vec3(k_maxRadiance));

	// The resolve pass applies exposure, tonemapping and sRGB encoding.
	fragColor = vec4(shaded, albedo.a);
	entityID = v_EntityID;
}
