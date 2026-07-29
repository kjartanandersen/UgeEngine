/**
 * @file Material.h
 * @brief Surface materials: a shader plus its texture maps and scalar properties.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Core/Core.h"

#include "Uge/Asset/Asset.h"
#include "Shader.h"

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
	 * @brief Scalar and colour factors applied alongside — or instead of — the texture maps.
	 * @ingroup group_renderer
	 */
	struct MaterialProperties
	{
		glm::vec4 AlbedoColor = glm::vec4(1.0f); ///< Base colour, multiplied with the albedo map.
		float Roughness = 1.0f; ///< Surface roughness in `[0, 1]`; `0` is mirror-smooth.
		float Metallic = 0.0f; ///< Metalness in `[0, 1]`; `1` is fully metallic.
		float EmissiveStrength = 0.0f; ///< Emission multiplier; `0` disables emission.
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
		MaterialTextureMap m_textureMaps; ///< Texture maps bound when this material is used.
		MaterialProperties m_properties; ///< Scalar and colour factors.

		Ref<Shader> m_shader; ///< Shader this material parameterizes.

	};

}