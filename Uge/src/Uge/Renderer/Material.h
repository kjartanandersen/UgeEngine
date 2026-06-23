#pragma once

#include "Uge/Core/Core.h"

#include "Uge/Asset/Asset.h"
#include "Shader.h"

#include <glm/glm.hpp>

namespace Uge
{

	struct MaterialTextureMap
	{
		AssetHandle Albedo = 0;
		AssetHandle Normal = 0;
		AssetHandle Roughness = 0;
		AssetHandle Metallic = 0;
		AssetHandle AmbientOcclusion = 0;
		AssetHandle Emissive = 0;
	};

	struct MaterialProperties
	{
		glm::vec4 AlbedoColor = glm::vec4(1.0f);
		float Roughness = 1.0f;
		float Metallic = 0.0f;
		float EmissiveStrength = 0.0f;
	};

	class Material : public Asset
	{

	public:
		static Ref<Material> Create(const Ref<Shader>& shader);

		virtual ~Material() = default;

		virtual void Bind() = 0;

		virtual Ref<Shader> GetShader() const = 0;
		virtual void SetShader(const Ref<Shader>& shader) = 0;

		AssetHandle GetAlbedoMap() const { return m_textureMaps.Albedo; }
		void SetAlbedoMap(AssetHandle handle) { m_textureMaps.Albedo = handle; }

		AssetHandle GetNormalMap() const { return m_textureMaps.Normal; }
		void SetNormalMap(AssetHandle handle) { m_textureMaps.Normal = handle; }

		AssetHandle GetRoughnessMap() const { return m_textureMaps.Roughness; }
		void SetRoughnessMap(AssetHandle handle) { m_textureMaps.Roughness = handle; }

		AssetHandle GetMetallicMap() const { return m_textureMaps.Metallic; }
		void SetMetallicMap(AssetHandle handle) { m_textureMaps.Metallic = handle; }

		AssetHandle GetAmbientOcclusionMap() const { return m_textureMaps.AmbientOcclusion; }
		void SetAmbientOcclusionMap(AssetHandle handle) { m_textureMaps.AmbientOcclusion = handle; }

		AssetHandle GetEmissiveMap() const { return m_textureMaps.Emissive; }
		void SetEmissiveMap(AssetHandle handle) { m_textureMaps.Emissive = handle; }

		static AssetType GetStaticType() { return AssetType::Material; }
		AssetType GetType() const override { return GetStaticType(); }


	protected:
		MaterialTextureMap m_textureMaps;
		MaterialProperties m_properties;

		Ref<Shader> m_shader;

	private:


	};

}