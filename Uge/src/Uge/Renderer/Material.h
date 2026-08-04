/**
 * @file Material.h
 * @brief Surface materials: a shader plus its texture maps and scalar properties.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Core/Core.h"

#include "Uge/Asset/Asset.h"
#include "Shader.h"
#include "UniformBuffer.h"

#include <cstdint>

#include <glm/glm.hpp>

namespace Uge
{

	/**
	 * @brief The set of texture maps a material can bind, referenced by handle.
	 * @ingroup group_renderer
	 *
	 * A handle of `0` means "no texture", in which case the corresponding scalar in
	 * Uge::MaterialProperties is used instead. These are the standard physically-based
	 * rendering inputs.
	 */
	struct MaterialTextureMap
	{
		AssetHandle Albedo = 0; ///< Base colour texture.
		AssetHandle Normal = 0; ///< Tangent-space normal map.
		AssetHandle Roughness = 0; ///< Per-texel surface roughness.
		AssetHandle Metallic = 0; ///< Per-texel metalness.
		AssetHandle AmbientOcclusion = 0; ///< Baked ambient occlusion.
		AssetHandle Emissive = 0; ///< Self-illumination colour.
	};

	/**
	 * @brief How a material's alpha channel is interpreted.
	 * @ingroup group_renderer
	 *
	 * Mirrors glTF's `alphaMode`, which is where the values come from on import.
	 * Uge::Model uses this to split its submeshes into an opaque pass and a
	 * back-to-front blended pass.
	 */
	enum class AlphaMode : int32_t
	{
		Opaque = 0, ///< Alpha ignored; the surface is fully opaque.
		Mask = 1,   ///< Fragments below Uge::MaterialProperties::AlphaCutoff are discarded.
		Blend = 2   ///< Alpha-blended over what is already in the target.
	};

	/**
	 * @brief Which texture maps a material actually has, as a bitfield.
	 * @ingroup group_renderer
	 *
	 * Uploaded to the shader so it can pick between a map and the matching scalar in
	 * Uge::MaterialProperties without a separate sampler test per surface.
	 */
	enum MaterialMapFlags : int32_t
	{
		MaterialMap_None = 0,                  ///< No maps.
		MaterialMap_Albedo = 1 << 0,           ///< Base colour map present.
		MaterialMap_Normal = 1 << 1,           ///< Normal map present.
		MaterialMap_Roughness = 1 << 2,        ///< Roughness map present.
		MaterialMap_Metallic = 1 << 3,         ///< Metallic map present.
		MaterialMap_AmbientOcclusion = 1 << 4, ///< Ambient occlusion map present.
		MaterialMap_Emissive = 1 << 5,         ///< Emissive map present.
		/**
		 * @brief Roughness and metalness share one texture, glTF style.
		 *
		 * glTF packs them into a single `metallicRoughnessTexture` with roughness in green
		 * and metalness in blue, and assimp reports that one file under both texture types.
		 * When this is set the shader reads those two channels of the roughness slot;
		 * otherwise it treats the two maps as separate single-channel textures.
		 */
		MaterialMap_PackedMetallicRoughness = 1 << 6,
		/**
		 * @brief An environment is bound for image-based lighting.
		 *
		 * Unlike every other flag here this is not a property of the material. It is set per
		 * frame by Uge::Model when the scene has a sky light, so that a material drawn
		 * without one falls back to a flat ambient term instead of sampling three cubemap
		 * slots that were never bound.
		 */
		MaterialMap_Environment = 1 << 7
	};

	/**
	 * @brief Scalar and colour factors applied alongside — or instead of — the texture maps.
	 * @ingroup group_renderer
	 */
	struct MaterialProperties
	{
		glm::vec4 AlbedoColor = glm::vec4(1.0f); ///< Base colour, multiplied with the albedo map.
		glm::vec3 EmissiveColor = glm::vec3(0.0f); ///< Emission colour, multiplied with the emissive map.
		float Roughness = 1.0f; ///< Surface roughness in `[0, 1]`; `0` is mirror-smooth.
		float Metallic = 0.0f; ///< Metalness in `[0, 1]`; `1` is fully metallic.
		float EmissiveStrength = 0.0f; ///< Emission multiplier; `0` disables emission.
		float AlphaCutoff = 0.5f; ///< Discard threshold, used only by Uge::AlphaMode::Mask.
		AlphaMode BlendMode = AlphaMode::Opaque; ///< How the alpha channel is interpreted.
	};

	/**
	 * @brief The material block uploaded to the GPU, laid out to match the shader.
	 * @ingroup group_renderer
	 *
	 * Corresponds to `layout(std140, binding = 2) uniform MaterialData` in
	 * `assets/shaders/Model.glsl`. Built by Uge::Material::BuildUniformData; the trailing
	 * padding is what rounds the block up to the 16-byte multiple `std140` requires.
	 *
	 * @warning Changing a field here means changing the shader block to match. A mismatch
	 * produces wrong values rather than an error.
	 */
	struct MaterialUniformData
	{
		glm::vec4 AlbedoColor;   ///< Base colour factor, including alpha.
		glm::vec4 EmissiveColor; ///< Emission colour factor; the `w` component is unused padding.
		float Roughness;         ///< Roughness factor.
		float Metallic;          ///< Metallic factor.
		float EmissiveStrength;  ///< Emission multiplier.
		float AlphaCutoff;       ///< Mask threshold.
		int32_t MapFlags;        ///< Bitwise OR of Uge::MaterialMapFlags.
		int32_t BlendMode;       ///< Uge::AlphaMode as an integer.
		int32_t EnvironmentMipCount; ///< Mip levels in the prefiltered map; scales the roughness lookup.
		float EnvironmentIntensity;  ///< Multiplier on the environment's contribution.
	};

	/**
	 * @brief A shader together with the textures and constants that parameterize it.
	 * @ingroup group_renderer
	 *
	 * An Uge::Asset, so meshes reference materials by handle. Materials synthesized during
	 * model import have no file on disk and are registered as memory-only assets, which
	 * means they are deliberately absent from the serialized asset registry.
	 *
	 * @see group_asset, Uge::MeshImporter
	 */
	class Material : public Asset
	{

	public:
		/**
		 * @brief Creates a material for the active graphics API.
		 * @param shader Shader this material parameterizes.
		 * @return The backend's material implementation.
		 */
		static Ref<Material> Create(const Ref<Shader>& shader);

		/** @brief Virtual destructor. */
		virtual ~Material() = default;

		/**
		 * @brief Binds the shader, its texture maps and its property uniforms.
		 *
		 * Resolves each texture handle through Uge::AssetManager, so a missing asset falls
		 * back rather than failing the draw.
		 */
		virtual void Bind() = 0;

		/**
		 * @brief The shader this material uses.
		 * @return The shader program.
		 */
		virtual Ref<Shader> GetShader() const = 0;
		/**
		 * @brief Replaces the shader.
		 * @param shader New shader program.
		 */
		virtual void SetShader(const Ref<Shader>& shader) = 0;

		/** @brief The albedo map. @return Texture handle, or `0` if unset. */
		AssetHandle GetAlbedoMap() const { return m_textureMaps.Albedo; }
		/** @brief Sets the albedo map. @param handle Texture handle, or `0` to clear. */
		void SetAlbedoMap(AssetHandle handle) { m_textureMaps.Albedo = handle; }

		/** @brief The normal map. @return Texture handle, or `0` if unset. */
		AssetHandle GetNormalMap() const { return m_textureMaps.Normal; }
		/** @brief Sets the normal map. @param handle Texture handle, or `0` to clear. */
		void SetNormalMap(AssetHandle handle) { m_textureMaps.Normal = handle; }

		/** @brief The roughness map. @return Texture handle, or `0` if unset. */
		AssetHandle GetRoughnessMap() const { return m_textureMaps.Roughness; }
		/** @brief Sets the roughness map. @param handle Texture handle, or `0` to clear. */
		void SetRoughnessMap(AssetHandle handle) { m_textureMaps.Roughness = handle; }

		/** @brief The metallic map. @return Texture handle, or `0` if unset. */
		AssetHandle GetMetallicMap() const { return m_textureMaps.Metallic; }
		/** @brief Sets the metallic map. @param handle Texture handle, or `0` to clear. */
		void SetMetallicMap(AssetHandle handle) { m_textureMaps.Metallic = handle; }

		/** @brief The ambient occlusion map. @return Texture handle, or `0` if unset. */
		AssetHandle GetAmbientOcclusionMap() const { return m_textureMaps.AmbientOcclusion; }
		/** @brief Sets the ambient occlusion map. @param handle Texture handle, or `0` to clear. */
		void SetAmbientOcclusionMap(AssetHandle handle) { m_textureMaps.AmbientOcclusion = handle; }

		/** @brief The emissive map. @return Texture handle, or `0` if unset. */
		AssetHandle GetEmissiveMap() const { return m_textureMaps.Emissive; }
		/** @brief Sets the emissive map. @param handle Texture handle, or `0` to clear. */
		void SetEmissiveMap(AssetHandle handle) { m_textureMaps.Emissive = handle; }

		/**
		 * @brief All texture maps at once.
		 * @return Const reference to the texture map set.
		 */
		const MaterialTextureMap& GetTextureMaps() const { return m_textureMaps; }
		/**
		 * @brief Replaces every texture map.
		 * @param textureMaps New map set.
		 */
		void SetTextureMaps(const MaterialTextureMap& textureMaps) { m_textureMaps = textureMaps; }

		/**
		 * @brief The scalar and colour properties.
		 * @return Const reference to the properties.
		 */
		const MaterialProperties& GetProperties() const { return m_properties; }
		/**
		 * @brief Replaces the scalar and colour properties.
		 * @param properties New property values.
		 */
		void SetProperties(const MaterialProperties& properties) { m_properties = properties; }

		/**
		 * @brief How this material's alpha channel is interpreted.
		 * @return The blend mode, defaulting to Uge::AlphaMode::Opaque.
		 */
		AlphaMode GetBlendMode() const { return m_properties.BlendMode; }

		/**
		 * @brief Packs the properties and map flags into the GPU block.
		 * @return The block, ready to upload to PropertiesUniformBuffer().
		 *
		 * The map flags are derived from which texture handles are non-zero, so the shader
		 * never has to guess whether a bound sampler holds real data.
		 */
		MaterialUniformData BuildUniformData() const;

		/**
		 * @brief Uploads a neutral opaque white block, as if for a material with no maps.
		 *
		 * The uniform block persists between draws, so a mesh with no material would
		 * otherwise be shaded with whatever material happened to bind last.
		 *
		 * @warning Requires a live graphics context.
		 */
		static void BindDefaultProperties();

		/**
		 * @brief Records whether an environment is bound, for every material to include.
		 * @param hasEnvironment `true` once the scene's environment maps are bound.
		 * @param prefilteredMipCount Mip levels in the prefiltered map; ignored when
		 *        @p hasEnvironment is `false`.
		 * @param intensity Multiplier on the environment's contribution to lighting.
		 *
		 * Image-based lighting is a property of the scene, not of a material, but the shader
		 * reads both facts out of the material block — that block is already uploaded once per
		 * material, so folding them in costs nothing, where a separate per-frame block would
		 * be another binding point and another upload.
		 *
		 * Called by Uge::Model::BeginScene. A material bound while this is `false` falls back
		 * to a flat ambient term rather than sampling cubemap slots that hold nothing.
		 */
		static void SetEnvironmentState(bool hasEnvironment, uint32_t prefilteredMipCount, float intensity);

		/**
		 * @brief The asset type this class represents.
		 * @return Uge::AssetType::Material.
		 */
		static AssetType GetStaticType() { return AssetType::Material; }
		/**
		 * @brief This instance's asset type.
		 * @return Uge::AssetType::Material.
		 */
		AssetType GetType() const override { return GetStaticType(); }


	protected:
		/**
		 * @brief The uniform buffer every material uploads its block into.
		 * @return The shared buffer, created on first call.
		 *
		 * Deliberately one buffer rather than one per material: the backend binds a buffer
		 * to binding point 2 when it is created, so per-material buffers would leave
		 * whichever was constructed last permanently bound.
		 *
		 * @warning Requires a live graphics context, so call it only from Bind().
		 */
		static const Ref<UniformBuffer>& PropertiesUniformBuffer();

	protected:
		MaterialTextureMap m_textureMaps; ///< Texture maps bound when this material is used.
		MaterialProperties m_properties; ///< Scalar and colour factors.

		Ref<Shader> m_shader; ///< Shader this material parameterizes.

	private:
		static bool s_hasEnvironment;
		static uint32_t s_environmentMipCount;
		static float s_environmentIntensity;

	};

}