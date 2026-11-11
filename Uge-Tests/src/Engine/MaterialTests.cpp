/**
 * @file MaterialTests.cpp
 * @brief Tests for Uge::Material's state that needs no GPU.
 */

#include <ugpch.h>

#include "Uge/Renderer/Material.h"
#include "Uge/Renderer/Shader.h"

#include <gtest/gtest.h>

namespace
{
	/** @brief A shader with no GPU program behind it. */
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
}

TEST(MaterialTest, SetShaderReplacesTheShader)
{
	// Like the importer's materials, this one starts with no shader of its own.
	Uge::Ref<Uge::Material> material = Uge::Material::Create(nullptr);
	EXPECT_EQ(material->GetShader(), nullptr);

	Uge::Ref<Uge::Shader> shader = Uge::CreateRef<StubShader>();
	material->SetShader(shader);
	EXPECT_EQ(material->GetShader(), shader);

	material->SetShader(nullptr);
	EXPECT_EQ(material->GetShader(), nullptr);
}