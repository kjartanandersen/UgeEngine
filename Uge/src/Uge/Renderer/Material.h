/**
 * @file Material.h
 * @brief Materials: a shader, texture maps and constants.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Core/Core.h"

#include "Uge/Asset/Asset.h"
#include "Shader.h"
#include "UniformBuffer.h"

#include <cstdint>
#include <array>

#include <glm/glm.hpp>


namespace Uge
{
	class Texture2D;

	/**
	 * @brief A material's texture maps, by handle. `0` means no texture.
	 * @ingroup group_renderer
	 */
	struct MaterialTextureMap
	{
		AssetHandle Albedo = 0; ///< Base colour.
		AssetHandle Normal = 0; ///< Tangent-space normals.
		AssetHandle Roughness = 0; ///< Roughness.
		AssetHandle Metallic = 0; ///< Metalness.
		AssetHandle AmbientOcclusion = 0; ///< Ambient occlusion.
		AssetHandle Emissive = 0; ///< Emitted colour.
	};

	/**
	 * @brief How a material uses alpha. The same modes as glTF's `alphaMode`.
	 * @ingroup group_renderer
	 */
	enum class AlphaMode : int32_t
	{
		Opaque = 0, ///< Alpha is ignored.
		Mask = 1,   ///< Fragments below Uge::MaterialProperties::AlphaCutoff are discarded.
		Blend = 2   ///< Blended over the target.
	};

	/**
	 * @brief Bits telling the shader which maps a material has.
	 * @ingroup group_renderer
	 */
	enum MaterialMapFlags : int32_t
	{
		MaterialMap_None = 0,
		MaterialMap_Albedo = 1 << 0,
		MaterialMap_Normal = 1 << 1,
		MaterialMap_Roughness = 1 << 2,
		MaterialMap_Metallic = 1 << 3,
		MaterialMap_AmbientOcclusion = 1 << 4,
		MaterialMap_Emissive = 1 << 5,
		/**
		 * @brief Roughness and metalness are one glTF texture: roughness in green, metalness
		 * in blue.
		 */
		MaterialMap_PackedMetallicRoughness = 1 << 6,
		/**
		 * @brief An environment is bound for image-based lighting. Set per frame, not per
		 * material; without it the shader uses a flat ambient term.
		 */
		MaterialMap_Environment = 1 << 7
	};

	/**
	 * @brief A material's constants, used with or in place of its texture maps.
	 * @ingroup group_renderer
	 */
	struct MaterialProperties
	{
		glm::vec4 AlbedoColor = glm::vec4(1.0f); ///< Base colour, multiplied with the albedo map.
		glm::vec3 EmissiveColor = glm::vec3(0.0f); ///< Emitted colour, multiplied with the emissive map.
		float Roughness = 1.0f; ///< In `[0, 1]`; `0` is mirror-smooth.
		float Metallic = 0.0f; ///< In `[0, 1]`; `1` is metal.
		float EmissiveStrength = 0.0f; ///< Emission multiplier; `0` turns emission off.
		float AlphaCutoff = 0.5f; ///< Discard threshold for Uge::AlphaMode::Mask.
		AlphaMode BlendMode = AlphaMode::Opaque; ///< How alpha is used.
		bool DoubleSided = false; ///< Whether both faces are drawn and lit.
	};

	/**
	 * @brief The material's uniform block.
	 * @ingroup group_renderer
	 *
	 * Matches `MaterialData` (std140, binding 2) in `assets/shaders/Model.glsl`; change
	 * both together.
	 */
	struct MaterialUniformData
	{
		glm::vec4 AlbedoColor;   ///< Base colour and alpha.
		glm::vec4 EmissiveColor; ///< Emitted colour; `w` is unused.
		float Roughness;         ///< Roughness.
		float Metallic;          ///< Metalness.
		float EmissiveStrength;  ///< Emission multiplier.
		float AlphaCutoff;       ///< Mask threshold.
		int32_t MapFlags;        ///< Uge::MaterialMapFlags.
		int32_t BlendMode;       ///< Uge::AlphaMode.
		int32_t EnvironmentMipCount; ///< Mip levels in the prefiltered map.
		float EnvironmentIntensity;  ///< Environment light multiplier.
	};

	/**
	 * @brief A shader with its texture maps and constants.
	 * @ingroup group_renderer
	 */
	class Material : public Asset
	{

	public:
		/**
		 * @brief Creates a material.
		 * @param shader The shader, or null to leave binding one to the caller.
		 * @return The material.
		 */
		static Ref<Material> Create(const Ref<Shader>& shader);

		/** @brief Virtual destructor. */
		virtual ~Material() = default;

		/**
		 * @brief Binds the shader if there is one, the textures and the uniform block.
		 *
		 * The textures are looked up on the first Bind() after a map changes.
		 *
		 * A texture that doesn't resolve isn't bound, and its slot keeps what was there.
		 */
		virtual void Bind() = 0;

		/**
		 * @brief The shader Bind() binds.
		 * @return The shader, or null if the caller binds one. Imported materials have none.
		 */
		virtual Ref<Shader> GetShader() const = 0;
		/**
		 * @brief Sets the shader Bind() binds.
		 * @param shader The shader, or null to leave binding one to the caller.
		 */
		virtual void SetShader(const Ref<Shader>& shader) = 0;

		/** @brief The albedo map. @return Texture handle, or `0` if unset. */
		AssetHandle GetAlbedoMap() const { return m_textureMaps.Albedo; }
		/** @brief Sets the albedo map. @param handle Texture handle, or `0` to clear. */
		void SetAlbedoMap(AssetHandle handle) { m_textureMaps.Albedo = handle; m_texturesResolved = false; }

		/** @brief The normal map. @return Texture handle, or `0` if unset. */
		AssetHandle GetNormalMap() const { return m_textureMaps.Normal; }
		/** @brief Sets the normal map. @param handle Texture handle, or `0` to clear. */
		void SetNormalMap(AssetHandle handle) { m_textureMaps.Normal = handle; m_texturesResolved = false; }

		/** @brief The roughness map. @return Texture handle, or `0` if unset. */
		AssetHandle GetRoughnessMap() const { return m_textureMaps.Roughness; }
		/** @brief Sets the roughness map. @param handle Texture handle, or `0` to clear. */
		void SetRoughnessMap(AssetHandle handle) { m_textureMaps.Roughness = handle; m_texturesResolved = false; }

		/** @brief The metallic map. @return Texture handle, or `0` if unset. */
		AssetHandle GetMetallicMap() const { return m_textureMaps.Metallic; }
		/** @brief Sets the metallic map. @param handle Texture handle, or `0` to clear. */
		void SetMetallicMap(AssetHandle handle) { m_textureMaps.Metallic = handle; m_texturesResolved = false; }

		/** @brief The ambient occlusion map. @return Texture handle, or `0` if unset. */
		AssetHandle GetAmbientOcclusionMap() const { return m_textureMaps.AmbientOcclusion; }
		/** @brief Sets the ambient occlusion map. @param handle Texture handle, or `0` to clear. */
		void SetAmbientOcclusionMap(AssetHandle handle) { m_textureMaps.AmbientOcclusion = handle; m_texturesResolved = false; }

		/** @brief The emissive map. @return Texture handle, or `0` if unset. */
		AssetHandle GetEmissiveMap() const { return m_textureMaps.Emissive; }
		/** @brief Sets the emissive map. @param handle Texture handle, or `0` to clear. */
		void SetEmissiveMap(AssetHandle handle) { m_textureMaps.Emissive = handle; m_texturesResolved = false; }

		/**
		 * @brief All the texture maps.
		 * @return The maps.
		 */
		const MaterialTextureMap& GetTextureMaps() const { return m_textureMaps; }
		/**
		 * @brief Replaces all the texture maps.
		 * @param textureMaps The maps.
		 */
		void SetTextureMaps(const MaterialTextureMap& textureMaps) { m_textureMaps = textureMaps; m_texturesResolved = false; }

		/**
		 * @brief The constants.
		 * @return The properties.
		 */
		const MaterialProperties& GetProperties() const { return m_properties; }
		/**
		 * @brief Replaces the constants.
		 * @param properties The properties.
		 */
		void SetProperties(const MaterialProperties& properties) { m_properties = properties; }

		/**
		 * @brief How the material uses alpha.
		 * @return The mode; Uge::AlphaMode::Opaque by default.
		 */
		AlphaMode GetBlendMode() const { return m_properties.BlendMode; }

		/**
		 * @brief Builds the uniform block. A map's flag is set when its handle isn't `0`.
		 * @return The block.
		 */
		MaterialUniformData BuildUniformData() const;

		/**
		 * @brief Uploads the block of an opaque white material with no maps.
		 *
		 * Call it before drawing a mesh with no material, which would otherwise use the
		 * last material's block.
		 */
		static void BindDefaultProperties();

		/** @brief Releases the shared uniform buffer. */
		static void Shutdown();

		/**
		 * @brief Sets the environment state that every material's block carries.
		 * @param hasEnvironment Whether the environment maps are bound.
		 * @param prefilteredMipCount Mip levels in the prefiltered map.
		 * @param intensity Environment light multiplier.
		 */
		static void SetEnvironmentState(bool hasEnvironment, uint32_t prefilteredMipCount, float intensity);

		/**
		 * @brief The asset type.
		 * @return Uge::AssetType::Material.
		 */
		static AssetType GetStaticType() { return AssetType::Material; }
		/**
		 * @brief The asset type.
		 * @return Uge::AssetType::Material.
		 */
		AssetType GetType() const override { return GetStaticType(); }

		/**
		* @brief Whether both faces are drawn, with back faces lit like front faces.
		* @return MaterialProperties::DoubleSided.
		*/
		bool IsDoubleSided() const { return m_properties.DoubleSided; }

	public:

		/** @brief The number of texture maps, bound to slots 0 to 5 in Uge::MaterialTextureMap's order. */
		static constexpr uint32_t TextureMapCount = 6;


	protected:
		/**
		 * @brief The uniform buffer at binding 2, shared by all materials.
		 * @return The buffer, created on first call; needs a GL context.
		 */
		static const Ref<UniformBuffer>& PropertiesUniformBuffer();

		/**
		 * @brief The maps' textures, in Uge::MaterialTextureMap's order, which is also their slot order.
		 *
		 * Looked up in the asset manager on the first call after a map changes, and kept until
		 * the next change. A map that is `0`, or doesn't resolve, gives null.
		 *
		 * @return The textures.
		 */
		const std::array<Ref<Texture2D>, TextureMapCount>& ResolveTextures();

	protected:
		MaterialTextureMap m_textureMaps; ///< The texture maps.
		MaterialProperties m_properties; ///< The constants.

		Ref<Shader> m_shader; ///< The shader, or null.

	private:
		static bool s_hasEnvironment;
		static uint32_t s_environmentMipCount;
		static float s_environmentIntensity;

		std::array<Ref<Texture2D>, TextureMapCount> m_textures; // Filled by ResolveTextures().
		bool m_texturesResolved = false; // Cleared by the map setters.

	};

}