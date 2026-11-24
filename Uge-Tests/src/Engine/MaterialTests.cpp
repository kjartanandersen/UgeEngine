/**
 * @file MaterialTests.cpp
 * @brief Tests for Uge::Material, without a GPU.
 */

#include <ugpch.h>

#include "Uge/Renderer/Material.h"
#include "Uge/Renderer/Shader.h"

#include "TestSupport/MockTexture2D.h"
#include "TestSupport/TempProject.h"

#include "Uge/Asset/AssetManager.h"
#include "Uge/Asset/EditorAssetManager.h"
#include "Uge/Project/Project.h"
#include "Uge/Renderer/Texture.h"

#include <gtest/gtest.h>

namespace
{
	/** @brief A shader with no GPU program. */
	class StubShader : public Uge::Shader
	{
	public:
		void Bind() const override {}
		void Unbind() const override {}
		void SetInt(const std::string, int) override {}
		void SetIntArray(const std::string, int*, int) override {}
		void SetFloat(const std::string, float) override {}
		void SetFloat3(const std::string, const glm::vec3&) override {}
		void SetFloat4(const std::string, const glm::vec4&) override {}
		void SetMat4(const std::string, const glm::mat4&) override {}
		const std::string& GetName() const override { return m_name; }

	private:
		std::string m_name = "Stub";
	};

	/** @brief A material with no GPU side, whose texture lookup is public. */
	class ProbeMaterial : public Uge::Material
	{
	public:
		void Bind() override {}
		Uge::Ref<Uge::Shader> GetShader() const override { return nullptr; }
		void SetShader(const Uge::Ref<Uge::Shader>&) override {}

		using Uge::Material::ResolveTextures;
	};
}

TEST(MaterialTest, SetShaderReplacesTheShader)
{
	// No shader, like an imported material.
	Uge::Ref<Uge::Material> material = Uge::Material::Create(nullptr);
	EXPECT_EQ(material->GetShader(), nullptr);

	Uge::Ref<Uge::Shader> shader = Uge::CreateRef<StubShader>();
	material->SetShader(shader);
	EXPECT_EQ(material->GetShader(), shader);

	material->SetShader(nullptr);
	EXPECT_EQ(material->GetShader(), nullptr);
}

TEST(MaterialTest, LooksItsTexturesUpAgainAfterAMapChanges)
{
	UgeTests::TempProject project;
	Uge::EditorAssetManager& manager = *Uge::Project::GetActive()->GetEditorAssetManager();

	auto first = Uge::CreateRef<UgeTests::MockTexture2D>();
	auto second = Uge::CreateRef<UgeTests::MockTexture2D>();
	const Uge::AssetHandle firstHandle = manager.AddMemoryOnlyAsset(first);
	const Uge::AssetHandle secondHandle = manager.AddMemoryOnlyAsset(second);

	ProbeMaterial material;
	material.SetAlbedoMap(firstHandle);
	EXPECT_EQ(material.ResolveTextures()[0], first);
	EXPECT_EQ(material.ResolveTextures()[1], nullptr);

	material.SetAlbedoMap(secondHandle);
	EXPECT_EQ(material.ResolveTextures()[0], second);

	Uge::MaterialTextureMap maps;
	maps.Emissive = firstHandle;
	material.SetTextureMaps(maps);
	EXPECT_EQ(material.ResolveTextures()[0], nullptr);
	EXPECT_EQ(material.ResolveTextures()[5], first);
}

TEST(MaterialTest, KeepsItsTexturesUntilAMapChanges)
{
	UgeTests::TempProject project;
	Uge::EditorAssetManager& manager = *Uge::Project::GetActive()->GetEditorAssetManager();

	auto texture = Uge::CreateRef<UgeTests::MockTexture2D>();
	const Uge::AssetHandle handle = manager.AddMemoryOnlyAsset(texture);

	ProbeMaterial material;
	material.SetAlbedoMap(handle);
	ASSERT_EQ(material.ResolveTextures()[0], texture);

	// The texture no longer resolves, but the material doesn't look it up again.
	manager.DeleteAsset(handle);
	ASSERT_EQ(Uge::AssetManager::GetAsset<Uge::Texture2D>(handle), nullptr);
	EXPECT_EQ(material.ResolveTextures()[0], texture);
}