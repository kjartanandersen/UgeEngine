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

TEST_F(SceneSerializerTest, AcceptsBooleansWrittenAsIntegers)
{
	// FirstGame's physicsScene.uge was written by 6e51012, whose mesh-collider writer
	// emitted its flags through (int) casts. yaml-cpp's bool conversion rejects "1".
	const std::filesystem::path path = m_project.WriteAssetFile("Scenes/IntBools.uge",
		"Scene: IntBools\n"
		"Entities:\n"
		"  - Entity: 42\n"
		"    TagComponent:\n"
		"      Tag: Collider\n"
		"    MeshColliderComponent:\n"
		"      Convex: 0\n"
		"      IsTrigger: 1\n");

	Uge::Ref<Scene> loaded = Uge::CreateRef<Scene>();
	ASSERT_TRUE(SceneSerializer(loaded).DeSerialize(path));

	Entity restored = loaded->FindEntityByName("Collider");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::MeshColliderComponent>());

	// Both differ from the component's defaults (Convex = true, IsTrigger = false), so a
	// pass means the values were actually read rather than left alone.
	const Uge::MeshColliderComponent& collider = restored.GetComponent<Uge::MeshColliderComponent>();
	EXPECT_FALSE(collider.Convex);
	EXPECT_TRUE(collider.IsTrigger);
}

TEST_F(SceneSerializerTest, AMistypedValueFailsTheLoadWithoutThrowing)
{
	const std::filesystem::path path = m_project.WriteAssetFile("Scenes/Mistyped.uge",
		"Scene: Mistyped\n"
		"Entities:\n"
		"  - Entity: 7\n"
		"    TagComponent:\n"
		"      Tag: Light\n"
		"    DirectionalLightComponent:\n"
		"      Intensity: bright\n");

	Uge::Ref<Scene> loaded = Uge::CreateRef<Scene>();

	bool result = true;
	EXPECT_NO_THROW({ result = SceneSerializer(loaded).DeSerialize(path); });
	EXPECT_FALSE(result);
}

TEST_F(SceneSerializerTest, AScriptComponentWithAnUnknownClassStillSaves)
{
	// Tests never initialise the script engine, so no script class is known at all. Saving
	// still keeps the component and its class name; it just writes no fields.
	Entity entity = m_scene->CreateEntity("Scripted");
	entity.AddComponent<Uge::ScriptComponent>().ClassName = "Game.DoesNotExist";

	Uge::Ref<Scene> loaded = RoundTrip();

	EXPECT_TRUE(m_deserialized);
	Entity restored = loaded->FindEntityByName("Scripted");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::ScriptComponent>());
	EXPECT_EQ(restored.GetComponent<Uge::ScriptComponent>().ClassName, "Game.DoesNotExist");
}

TEST_F(SceneSerializerTest, AMeshComponentWithoutAHandleHasNoMesh)
{
	// Scenes from before asset handles describe a mesh by path, with no MeshHandle key. The
	// component is kept with no mesh; 0 means "none".
	const std::filesystem::path path = m_project.WriteAssetFile("Scenes/OldMesh.uge",
		"Scene: OldMesh\n"
		"Entities:\n"
		"  - Entity: 5\n"
		"    TagComponent:\n"
		"      Tag: Backpack\n"
		"    MeshComponent:\n"
		"      Path: Mesh/backpack/scene.gltf\n");

	Uge::Ref<Scene> loaded = Uge::CreateRef<Scene>();
	ASSERT_TRUE(SceneSerializer(loaded).DeSerialize(path));

	Entity restored = loaded->FindEntityByName("Backpack");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::MeshComponent>());
	EXPECT_EQ((uint64_t)restored.GetComponent<Uge::MeshComponent>().Mesh, 0ull);
}

namespace
{
	/** @brief A material with no default values, so one that isn't read back shows up. */
	Uge::PhysicsMaterial DistinctMaterial()
	{
		Uge::PhysicsMaterial material;
		material.Friction = 0.7f;
		material.Restitution = 0.25f;
		material.Density = 500.0f;
		return material;
	}

	void ExpectDistinctMaterial(const Uge::PhysicsMaterial& material)
	{
		EXPECT_NEAR(material.Friction, 0.7f, 1e-5f);
		EXPECT_NEAR(material.Restitution, 0.25f, 1e-5f);
		EXPECT_NEAR(material.Density, 500.0f, 1e-3f);
	}

	void ExpectSameVector(const glm::vec3& actual, const glm::vec3& expected)
	{
		EXPECT_NEAR(actual.x, expected.x, 1e-5f);
		EXPECT_NEAR(actual.y, expected.y, 1e-5f);
		EXPECT_NEAR(actual.z, expected.z, 1e-5f);
	}
}

TEST_F(SceneSerializerTest, PreservesRigidbodies)
{
	Entity entity = m_scene->CreateEntity("Crate");
	Uge::RigidbodyComponent& body = entity.AddComponent<Uge::RigidbodyComponent>();
	body.Type = Uge::BodyType::Kinematic;
	body.Layer = Uge::PhysicsLayer::Static;
	body.Mass = 3.5f;
	body.LinearDamping = 0.2f;
	body.AngularDamping = 0.3f;
	body.GravityFactor = 0.5f;
	body.FixedRotation = true;

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->FindEntityByName("Crate");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::RigidbodyComponent>());

	const Uge::RigidbodyComponent& result = restored.GetComponent<Uge::RigidbodyComponent>();
	EXPECT_EQ(result.Type, Uge::BodyType::Kinematic);
	EXPECT_EQ(result.Layer, Uge::PhysicsLayer::Static);
	EXPECT_NEAR(result.Mass, 3.5f, 1e-5f);
	EXPECT_NEAR(result.LinearDamping, 0.2f, 1e-5f);
	EXPECT_NEAR(result.AngularDamping, 0.3f, 1e-5f);
	EXPECT_NEAR(result.GravityFactor, 0.5f, 1e-5f);
	EXPECT_TRUE(result.FixedRotation);
}

TEST_F(SceneSerializerTest, PreservesBoxColliders)
{
	Entity entity = m_scene->CreateEntity("Wall");
	Uge::BoxColliderComponent& box = entity.AddComponent<Uge::BoxColliderComponent>();
	box.Offset = { 1.0f, 2.0f, 3.0f };
	box.HalfExtents = { 0.5f, 1.5f, 2.5f };
	box.Material = DistinctMaterial();
	box.IsTrigger = true;

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->FindEntityByName("Wall");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::BoxColliderComponent>());

	const Uge::BoxColliderComponent& result = restored.GetComponent<Uge::BoxColliderComponent>();
	ExpectSameVector(result.Offset, { 1.0f, 2.0f, 3.0f });
	ExpectSameVector(result.HalfExtents, { 0.5f, 1.5f, 2.5f });
	ExpectDistinctMaterial(result.Material);
	EXPECT_TRUE(result.IsTrigger);
}

TEST_F(SceneSerializerTest, PreservesSphereColliders)
{
	Entity entity = m_scene->CreateEntity("Ball");
	Uge::SphereColliderComponent& sphere = entity.AddComponent<Uge::SphereColliderComponent>();
	sphere.Offset = { -1.0f, 0.5f, 2.0f };
	sphere.Radius = 1.25f;
	sphere.Material = DistinctMaterial();
	sphere.IsTrigger = true;

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->FindEntityByName("Ball");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::SphereColliderComponent>());

	const Uge::SphereColliderComponent& result = restored.GetComponent<Uge::SphereColliderComponent>();
	ExpectSameVector(result.Offset, { -1.0f, 0.5f, 2.0f });
	EXPECT_NEAR(result.Radius, 1.25f, 1e-5f);
	ExpectDistinctMaterial(result.Material);
	EXPECT_TRUE(result.IsTrigger);
}

TEST_F(SceneSerializerTest, PreservesCapsuleColliders)
{
	Entity entity = m_scene->CreateEntity("Pill");
	Uge::CapsuleColliderComponent& capsule = entity.AddComponent<Uge::CapsuleColliderComponent>();
	capsule.Offset = { 0.0f, 1.0f, 0.0f };
	capsule.Radius = 0.4f;
	capsule.HalfHeight = 1.75f; // not a whole number, so any rounding on the way shows
	capsule.Material = DistinctMaterial();
	capsule.IsTrigger = true;

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->FindEntityByName("Pill");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::CapsuleColliderComponent>());

	const Uge::CapsuleColliderComponent& result = restored.GetComponent<Uge::CapsuleColliderComponent>();
	ExpectSameVector(result.Offset, { 0.0f, 1.0f, 0.0f });
	EXPECT_NEAR(result.Radius, 0.4f, 1e-5f);
	EXPECT_NEAR(result.HalfHeight, 1.75f, 1e-5f);
	ExpectDistinctMaterial(result.Material);
	EXPECT_TRUE(result.IsTrigger);
}

TEST_F(SceneSerializerTest, PreservesMeshColliders)
{
	// The mesh is a handle; the model behind it doesn't need to exist for the handle to be
	// saved and loaded.
	Entity entity = m_scene->CreateEntity("Rock");
	Uge::MeshColliderComponent& collider = entity.AddComponent<Uge::MeshColliderComponent>();
	collider.Mesh = Uge::AssetHandle(1234567ull);
	collider.Convex = false;
	collider.Offset = { 2.0f, 0.0f, -1.0f };
	collider.Material = DistinctMaterial();
	collider.IsTrigger = true;

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->FindEntityByName("Rock");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::MeshColliderComponent>());

	const Uge::MeshColliderComponent& result = restored.GetComponent<Uge::MeshColliderComponent>();
	EXPECT_EQ((uint64_t)result.Mesh, 1234567ull);
	EXPECT_FALSE(result.Convex);
	ExpectSameVector(result.Offset, { 2.0f, 0.0f, -1.0f });
	ExpectDistinctMaterial(result.Material);
	EXPECT_TRUE(result.IsTrigger);
}

TEST_F(SceneSerializerTest, PreservesSkyLights)
{
	Entity entity = m_scene->CreateEntity("Sky");
	Uge::SkyLightComponent& sky = entity.AddComponent<Uge::SkyLightComponent>();
	sky.Environment = Uge::AssetHandle(7654321ull);
	sky.Intensity = 2.5f;

	Uge::Ref<Scene> loaded = RoundTrip();

	Entity restored = loaded->FindEntityByName("Sky");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::SkyLightComponent>());

	const Uge::SkyLightComponent& result = restored.GetComponent<Uge::SkyLightComponent>();
	EXPECT_EQ((uint64_t)result.Environment, 7654321ull);
	EXPECT_NEAR(result.Intensity, 2.5f, 1e-5f);
}

TEST_F(SceneSerializerTest, AScriptComponentWithNoClassKeepsItsEmptyName)
{
	// A Script component just added in the editor has no class yet. Saving and loading
	// keeps the component, with the class still empty.
	Entity entity = m_scene->CreateEntity("Unassigned");
	entity.AddComponent<Uge::ScriptComponent>();

	Uge::Ref<Scene> loaded = RoundTrip();
	ASSERT_TRUE(m_deserialized);

	Entity restored = loaded->FindEntityByName("Unassigned");
	ASSERT_TRUE(restored);
	ASSERT_TRUE(restored.HasComponent<Uge::ScriptComponent>());
	EXPECT_TRUE(restored.GetComponent<Uge::ScriptComponent>().ClassName.empty());
}
