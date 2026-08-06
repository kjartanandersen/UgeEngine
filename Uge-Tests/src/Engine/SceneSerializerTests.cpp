/**
 * @file SceneSerializerTests.cpp
 * @brief Round-trip tests for Uge::SceneSerializer and the `.uge` scene format.
 */

#include <ugpch.h>

#include "TestSupport/TempProject.h"

#include "Uge/Project/Project.h"
#include "Uge/Scene/Components.h"
#include "Uge/Scene/Entity.h"
#include "Uge/Scene/Scene.h"
#include "Uge/Scene/SceneSerializer.h"

#include <gtest/gtest.h>

#include <fstream>

using Uge::Entity;
using Uge::Project;
using Uge::Scene;
using Uge::SceneSerializer;
using UgeTests::TempProject;

namespace
{

	/** @brief A project to serialize into, plus a scene to serialize. */
	class SceneSerializerTest : public ::testing::Test
	{
	protected:
		void SetUp() override { m_scene = Uge::CreateRef<Scene>(); }

		/** @brief Writes m_scene out and reads it back into a fresh scene. */
		Uge::Ref<Scene> RoundTrip(const std::filesystem::path& relativePath = "Scenes/Test.uge")
		{
			const std::filesystem::path path = Project::GetAssetFileSystemPath(relativePath);
			std::filesystem::create_directories(path.parent_path());

			SceneSerializer(m_scene).Serialize(path);

			Uge::Ref<Scene> loaded = Uge::CreateRef<Scene>();
			m_deserialized = SceneSerializer(loaded).DeSerialize(path);

			return loaded;
		}

		TempProject m_project;
		Uge::Ref<Scene> m_scene;
		bool m_deserialized = false;
	};

}

TEST_F(SceneSerializerTest, WritesAFileThatCanBeReadBack)
{
	m_scene->CreateEntity("Entity");

	Uge::Ref<Scene> loaded = RoundTrip();

	EXPECT_TRUE(m_deserialized);
	EXPECT_EQ(loaded->GetAllEntitiesWith<Uge::IDComponent>().size(), 1u);
}

TEST_F(SceneSerializerTest, AnEmptySceneRoundTrips)
{
	Uge::Ref<Scene> loaded = RoundTrip();

	EXPECT_TRUE(m_deserialized);
	EXPECT_EQ(loaded->GetAllEntitiesWith<Uge::IDComponent>().size(), 0u);
}

TEST_F(SceneSerializerTest, PreservesEntityIdentity)
{
	// Everything that refers to an entity across a save — the editor's selection, a script
	// holding a handle — goes through the UUID, so a regenerated id breaks all of it.
	Entity original = m_scene->CreateEntity("Player");
	const Uge::UUID id = original.GetUUID();

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->GetEntityByUUID(id);
	ASSERT_TRUE(restored);
	EXPECT_EQ(restored.GetName(), "Player");
}

TEST_F(SceneSerializerTest, PreservesTransforms)
{
	Entity entity = m_scene->CreateEntity("Crate");
	Uge::TransformComponent& transform = entity.GetComponent<Uge::TransformComponent>();
	transform.Translation = glm::vec3(1.5f, -2.25f, 3.0f);
	transform.Rotation = glm::vec3(0.1f, 0.2f, 0.3f);
	transform.Scale = glm::vec3(2.0f, 0.5f, 1.0f);

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->FindEntityByName("Crate");
	ASSERT_TRUE(restored);

	const Uge::TransformComponent& result = restored.GetComponent<Uge::TransformComponent>();
	EXPECT_NEAR(result.Translation.x, 1.5f, 1e-5f);
	EXPECT_NEAR(result.Translation.y, -2.25f, 1e-5f);
	EXPECT_NEAR(result.Translation.z, 3.0f, 1e-5f);
	EXPECT_NEAR(result.Rotation.y, 0.2f, 1e-5f);
	EXPECT_NEAR(result.Scale.x, 2.0f, 1e-5f);
	EXPECT_NEAR(result.Scale.y, 0.5f, 1e-5f);
}

TEST_F(SceneSerializerTest, PreservesCameraSettings)
{
	Entity entity = m_scene->CreateEntity("Camera");
	Uge::CameraComponent& camera = entity.AddComponent<Uge::CameraComponent>();
	camera.Primary = true;
	camera.FixedAspectRatio = true;
	camera.Cam.SetProjectionType(Uge::SceneCamera::ProjectionType::Perspective);
	camera.Cam.SetPerspVerticalFOV(70.0f);
	camera.Cam.SetPerspNearClip(0.25f);
	camera.Cam.SetPerspFarClip(750.0f);

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->FindEntityByName("Camera");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::CameraComponent>());

	const Uge::CameraComponent& result = restored.GetComponent<Uge::CameraComponent>();
	EXPECT_TRUE(result.Primary);
	EXPECT_TRUE(result.FixedAspectRatio);
	EXPECT_EQ(result.Cam.GetProjectionType(), Uge::SceneCamera::ProjectionType::Perspective);
	EXPECT_NEAR(result.Cam.GetPerspVerticalFOV(), 70.0f, 1e-3f);
	EXPECT_NEAR(result.Cam.GetPerspNearClip(), 0.25f, 1e-5f);
	EXPECT_NEAR(result.Cam.GetPerspFarClip(), 750.0f, 1e-3f);
}

TEST_F(SceneSerializerTest, PreservesSpriteRenderers)
{
	Entity entity = m_scene->CreateEntity("Sprite");
	Uge::SpriteRendererComponent& sprite = entity.AddComponent<Uge::SpriteRendererComponent>();
	sprite.Color = glm::vec4(0.1f, 0.2f, 0.3f, 0.4f);
	sprite.TilingFactor = 3.5f;
	sprite.Texture = Uge::AssetHandle(0xABCDEF);

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->FindEntityByName("Sprite");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::SpriteRendererComponent>());

	const Uge::SpriteRendererComponent& result = restored.GetComponent<Uge::SpriteRendererComponent>();
	EXPECT_NEAR(result.Color.r, 0.1f, 1e-5f);
	EXPECT_NEAR(result.Color.a, 0.4f, 1e-5f);
	EXPECT_NEAR(result.TilingFactor, 3.5f, 1e-5f);

	// Asset references are stored as handles, so they survive without the asset itself
	// being present.
	EXPECT_EQ((uint64_t)result.Texture, 0xABCDEFull);
}

TEST_F(SceneSerializerTest, PreservesMeshHandles)
{
	Entity entity = m_scene->CreateEntity("Model");
	entity.AddComponent<Uge::MeshComponent>().Mesh = Uge::AssetHandle(0x123456);

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->FindEntityByName("Model");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::MeshComponent>());
	EXPECT_EQ((uint64_t)restored.GetComponent<Uge::MeshComponent>().Mesh, 0x123456ull);
}

TEST_F(SceneSerializerTest, PreservesDirectionalLights)
{
	Entity entity = m_scene->CreateEntity("Sun");
	Uge::DirectionalLightComponent& light = entity.AddComponent<Uge::DirectionalLightComponent>();
	light.Color = glm::vec3(1.0f, 0.9f, 0.8f);
	light.Intensity = 4.5f;

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->FindEntityByName("Sun");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::DirectionalLightComponent>());

	const Uge::DirectionalLightComponent& result = restored.GetComponent<Uge::DirectionalLightComponent>();
	EXPECT_NEAR(result.Color.g, 0.9f, 1e-5f);
	EXPECT_NEAR(result.Intensity, 4.5f, 1e-5f);
}

TEST_F(SceneSerializerTest, PreservesTextComponents)
{
	Entity entity = m_scene->CreateEntity("Label");
	Uge::TextComponent& text = entity.AddComponent<Uge::TextComponent>();
	text.TextString = "Hello, Uge";
	text.Kerning = 0.5f;
	text.LineSpacing = 1.25f;
	text.Color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->FindEntityByName("Label");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::TextComponent>());

	const Uge::TextComponent& result = restored.GetComponent<Uge::TextComponent>();
	EXPECT_EQ(result.TextString, "Hello, Uge");
	EXPECT_NEAR(result.Kerning, 0.5f, 1e-5f);
	EXPECT_NEAR(result.LineSpacing, 1.25f, 1e-5f);
}

TEST_F(SceneSerializerTest, PreservesEveryEntityInTheScene)
{
	for (int i = 0; i < 25; i++)
	{
		m_scene->CreateEntity("Entity" + std::to_string(i));
	}

	Uge::Ref<Scene> loaded = RoundTrip();

	EXPECT_EQ(loaded->GetAllEntitiesWith<Uge::IDComponent>().size(), 25u);
	EXPECT_TRUE(loaded->FindEntityByName("Entity0"));
	EXPECT_TRUE(loaded->FindEntityByName("Entity24"));
}

TEST_F(SceneSerializerTest, ComponentsAreNotAddedToEntitiesThatDidNotHaveThem)
{
	m_scene->CreateEntity("Plain");

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->FindEntityByName("Plain");
	ASSERT_TRUE(restored);
	EXPECT_FALSE(restored.HasComponent<Uge::CameraComponent>());
	EXPECT_FALSE(restored.HasComponent<Uge::SpriteRendererComponent>());
	EXPECT_FALSE(restored.HasComponent<Uge::MeshComponent>());
}

TEST_F(SceneSerializerTest, TheSceneIsNamedAfterItsPathRelativeToTheContentRoot)
{
	Uge::Ref<Scene> loaded = RoundTrip("Scenes/Named.uge");

	EXPECT_NE(loaded->GetName().find("Named.uge"), std::string::npos);
}

TEST_F(SceneSerializerTest, DeserializingAFileThatIsNotASceneFails)
{
	const std::filesystem::path path = m_project.WriteAssetFile("NotAScene.uge",
		"SomethingElse:\n  key: value\n");

	Uge::Ref<Scene> loaded = Uge::CreateRef<Scene>();

	EXPECT_FALSE(SceneSerializer(loaded).DeSerialize(path));
	EXPECT_EQ(loaded->GetAllEntitiesWith<Uge::IDComponent>().size(), 0u);
}

TEST_F(SceneSerializerTest, DeserializingAMissingFileFailsWithoutThrowing)
{
	// YAML::BadFile, not a ParserException — the catch has to be wide enough to cover it
	// or this escapes every caller.
	Uge::Ref<Scene> loaded = Uge::CreateRef<Scene>();

	bool result = true;
	EXPECT_NO_THROW({
		result = SceneSerializer(loaded).DeSerialize(m_project.AssetDirectory() / "Gone.uge");
		});

	EXPECT_FALSE(result);
}

TEST_F(SceneSerializerTest, DeserializingMalformedYamlFails)
{
	const std::filesystem::path path = m_project.WriteAssetFile("Broken.uge",
		"Scene: [unterminated\n  - nonsense: {\n");

	Uge::Ref<Scene> loaded = Uge::CreateRef<Scene>();

	EXPECT_FALSE(SceneSerializer(loaded).DeSerialize(path));
}
