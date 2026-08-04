#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

// Binding 3, not 0: Renderer2D owns a camera block at binding 0 that is a bare mat4, and two
// differently sized blocks on one binding point let a draw read past the end of whichever
// buffer happens to be bound.
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

	// The fragment stage differentiates this to rebuild a tangent frame.
	v_WorldPosition = worldPosition.xyz;

	// Passed down rather than having the fragment stage read the camera block itself.
	// Declaring one uniform block in two stages gives each a differently generated name once
	// the shader round-trips through SPIR-V, which some drivers refuse to link.
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

// Image-based lighting, bound once per frame rather than per material. @see Uge::Environment
layout(binding = 6) uniform samplerCube u_IrradianceMap;
layout(binding = 7) uniform samplerCube u_PrefilterMap;
layout(binding = 8) uniform sampler2D u_BrdfLut;

// Mirrors Uge::MaterialUniformData. Keep the two in step: a layout mismatch shows up as
// wrong values, not as an error.
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
	int u_EnvironmentMipCount;
	float u_EnvironmentIntensity;
};

// The scene's single directional light. Mirrors Uge::LightData; a zero radiance means the
// scene has no light entity, and only the environment contributes.
layout(std140, binding = 4) uniform LightData
{
	vec4 u_LightDirection;
	vec4 u_LightRadiance;
};

// Uge::MaterialMapFlags. A slot whose flag is clear still holds the previous material's
// texture, so every read below has to be gated on these.
const int MAP_ALBEDO = 1 << 0;
const int MAP_NORMAL = 1 << 1;
const int MAP_ROUGHNESS = 1 << 2;
const int MAP_METALLIC = 1 << 3;
const int MAP_AMBIENT_OCCLUSION = 1 << 4;
const int MAP_EMISSIVE = 1 << 5;
const int MAP_PACKED_METALLIC_ROUGHNESS = 1 << 6;

// Not a material texture: set per frame when the scene has a sky light, so a material bound
// without an environment falls back rather than sampling three unbound cubemap slots.
const int MAP_ENVIRONMENT = 1 << 7;

// Uge::AlphaMode
const int ALPHA_MODE_MASK = 1;

// Shading happens in linear space and this shader writes linear radiance - the render target
// is RGBA16F, and the sRGB encode happens once for the whole scene in the resolve pass.
// @see Uge::PostProcess
//
// Colour textures are sRGB encoded, so they are decoded on the way in. Base colour and
// emissive are colour data; normal, roughness, metallic and ambient occlusion are not, and
// are sampled raw.
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

// Rebuilds a tangent frame from screen-space derivatives instead of vertex tangents.
// MeshVertex carries none, and generating them at import cost 430s against 6s on a dense
// mesh (see MeshImporter::ImportMesh), so the frame is derived per fragment here.
//
// This requires the OpenGL backend to compile GLSL source rather than ingest SPIR-V: Intel's
// GL_ARB_gl_spirv path rejects this function with an empty info log even though spirv-val
// accepts the module. @see OpenGLShader::CompileOrGetOpenGLSource
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

	// Degenerate UVs give a zero-length basis; the clamp keeps inversesqrt finite and
	// collapses the frame onto the interpolated normal instead of producing NaNs.
	float maxLengthSquared = max(max(dot(tangent, tangent), dot(bitangent, bitangent)), 1e-12);
	float invMax = inversesqrt(maxLengthSquared);

	return mat3(tangent * invMax, bitangent * invMax, normal);
}

const float k_pi = 3.14159265359;

// ---- Cook-Torrance microfacet BRDF ----
//
// Three terms over a 4*NdotL*NdotV denominator: D, how many microfacets are oriented to
// reflect light this way; G, how many of those are neither shadowed nor masked by their
// neighbours; F, how much each one actually reflects at this angle.

// Trowbridge-Reitz GGX. Roughness is squared before use so that the artist-facing value
// behaves perceptually linearly rather than bunching all the interesting change near zero.
float DistributionGGX(vec3 normal, vec3 halfway, float roughness)
{
	float a = roughness * roughness;
	float aSquared = a * a;
	float normalDotHalfway = max(dot(normal, halfway), 0.0);
	float denominator = normalDotHalfway * normalDotHalfway * (aSquared - 1.0) + 1.0;
	return aSquared / (k_pi * denominator * denominator);
}

// Schlick's approximation to Smith's geometry term, with the k for direct lighting:
// (roughness + 1)^2 / 8. The image-based path uses roughness^2 / 2 instead, which is why
// BrdfLut.glsl carries its own copy rather than sharing this one.
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

// The roughness-aware variant, for the ambient term where there is no half vector. Without
// the roughness clamp a rough surface picks up a bright white rim, because plain Schlick
// drives reflectance to 1 at grazing angles regardless of how scattered the surface is.
vec3 FresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
	vec3 clamped = max(vec3(1.0 - roughness), f0);
	return f0 + (clamped - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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
	if ((u_MapFlags & MAP_NORMAL) != 0)
	{
		vec3 tangentNormal = texture(u_NormalMap, v_TexCoord).xyz * 2.0 - 1.0;
		normal = normalize(CotangentFrame(normal, v_WorldPosition, v_TexCoord) * tangentNormal);
	}

	// glTF packs roughness into green and metalness into blue of one texture; other sources
	// supply two separate single-channel maps.
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

	// A roughness of exactly 0 makes the GGX denominator vanish when the half-vector lines up
	// with the normal, giving 0/0. The floor costs nothing visually - nothing real is a
	// perfect mirror - and keeps a NaN out of the render target.
	roughness = clamp(roughness, 0.025, 1.0);

	float ambientOcclusion = 1.0;
	if ((u_MapFlags & MAP_AMBIENT_OCCLUSION) != 0)
	{
		ambientOcclusion = texture(u_AmbientOcclusionMap, v_TexCoord).r;
	}

	// Already normalized and reversed on the CPU side, so this points from the surface towards
	// the light. Zero radiance means the scene has no directional light at all.
	vec3 lightDirection = u_LightDirection.xyz;
	vec3 lightRadiance = u_LightRadiance.rgb;

	vec3 viewDirection = normalize(v_ViewDirection);
	vec3 halfway = normalize(lightDirection + viewDirection);

	float normalDotView = max(dot(normal, viewDirection), 0.0);
	float normalDotLight = max(dot(normal, lightDirection), 0.0);

	// Reflectance at normal incidence. Dielectrics reflect about 4% regardless of colour and
	// keep their albedo for the diffuse term; metals have no diffuse at all and tint their
	// reflection with the albedo instead. Metalness blends between the two.
	vec3 f0 = mix(vec3(0.04), albedo.rgb, metallic);

	// ---- Direct lighting: Cook-Torrance ----

	float distribution = DistributionGGX(normal, halfway, roughness);
	float geometry = GeometrySmith(normalDotView, normalDotLight, roughness);
	vec3 fresnel = FresnelSchlick(max(dot(halfway, viewDirection), 0.0), f0);

	// The 4 * NdotL * NdotV denominator of the microfacet model. NdotL cancels against the
	// incident irradiance below, but is left in for clarity; the epsilon guards the grazing
	// case where both dot products approach zero.
	vec3 specular = (distribution * geometry * fresnel)
		/ (4.0 * normalDotView * normalDotLight + 0.0001);

	// A directional light has zero solid angle, so on a smooth surface its reflection is a
	// point of unbounded intensity - GGX peaks past 100000 on this car's paint. A real sun is
	// about half a degree across, which is what gives a real highlight a finite size and a
	// falloff; modelling that properly needs an area light. Until then, clamping keeps the
	// highlight from feeding the bloom pass a value thousands of times over the threshold and
	// blowing a hole in the image.
	const float k_maxDirectSpecular = 50.0;
	specular = min(specular, vec3(k_maxDirectSpecular));

	// Energy conservation: what is reflected specularly cannot also be refracted and come back
	// out as diffuse. Metals get no diffuse term at all.
	vec3 diffuseWeight = (vec3(1.0) - fresnel) * (1.0 - metallic);

	vec3 shaded = (diffuseWeight * albedo.rgb / k_pi + specular) * lightRadiance * normalDotLight;

	// ---- Ambient: image-based lighting ----

	if ((u_MapFlags & MAP_ENVIRONMENT) != 0)
	{
		// Diffuse half: irradiance already integrates the cosine-weighted hemisphere, so it is
		// one fetch rather than a loop. @see IrradianceConvolution.glsl
		vec3 irradiance = texture(u_IrradianceMap, normal).rgb;
		vec3 diffuseIBL = irradiance * albedo.rgb;

		// Specular half, Karis' split sum: the prefiltered environment supplies the incoming
		// radiance for this roughness, and the lookup table supplies the scale and bias to
		// apply to F0. Roughness selects the mip, which is why the map needs a full chain.
		vec3 reflected = reflect(-viewDirection, normal);
		float mipLevel = roughness * float(u_EnvironmentMipCount - 1);
		vec3 prefiltered = textureLod(u_PrefilterMap, reflected, mipLevel).rgb;

		vec2 environmentBrdf = texture(u_BrdfLut, vec2(normalDotView, roughness)).rg;
		vec3 specularIBL = prefiltered * (f0 * environmentBrdf.x + environmentBrdf.y);

		// Fresnel here uses the view-normal angle, not the half vector: there is no single
		// light direction to form a half vector from. The roughness term stops a rough surface
		// developing an unnaturally bright rim.
		vec3 ambientFresnel = FresnelSchlickRoughness(normalDotView, f0, roughness);
		vec3 ambientDiffuseWeight = (vec3(1.0) - ambientFresnel) * (1.0 - metallic);

		shaded += (ambientDiffuseWeight * diffuseIBL + specularIBL)
			* ambientOcclusion * u_EnvironmentIntensity;
	}
	else
	{
		// No environment bound. Scaled against the light rather than being an absolute constant:
		// sunlight has a radiance well above 1, so a fixed small fill left every surface facing
		// away from it effectively black - a hundred to one between the lit and unlit sides of
		// the same panel. The floor keeps a scene with no light at all from going fully black.
		// Still only a stand-in; a scene that cares wants a sky light.
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

	// Linear and high dynamic range, but not unbounded. GGX is a delta function in the limit:
	// on a smooth surface (the car's paint is roughness 0.04) an exactly mirror-aligned
	// half-vector evaluates to over 100000, which past 65504 becomes +Inf in the RGBA16F
	// target. Bloom then turns that into NaN - its Karis weight is 1/(1+luma), so Inf weighs
	// 0, and Inf * 0 is NaN - which spreads across the whole downsample pyramid as a black
	// rectangle. Clamping here leaves ample headroom above the bloom threshold of 1.0 while
	// keeping every value finite.
	const float k_maxRadiance = 1000.0;
	shaded = min(shaded, vec3(k_maxRadiance));

	// The resolve pass applies exposure, the tonemap curve and the sRGB encode; doing it here
	// would tonemap each surface in isolation and then again as a group.
	fragColor = vec4(shaded, albedo.a);
	entityID = v_EntityID;
}
