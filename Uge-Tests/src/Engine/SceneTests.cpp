/**
 * @file SceneTests.cpp
 * @brief Tests for Uge::Scene's entity operations.
 */

#include <ugpch.h>

#include "Uge/Scene/Components.h"
#include "Uge/Scene/Entity.h"
#include "Uge/Scene/Scene.h"
#include "Uge/Scripting/ScriptEngine.h"

#include "../TestSupport/PhysicsFixture.h"

#include <gtest/gtest.h>

#include <unordered_set>
#include <filesystem>
#include <vector>

using Uge::Entity;
using Uge::Scene;

namespace
{

	/** @brief An empty scene for each test. */
	class SceneTest : public ::testing::Test
	{
	protected:
		void SetUp() override { m_scene = Uge::CreateRef<Scene>(); }

		Uge::Ref<Scene> m_scene;
	};

}

TEST_F(SceneTest, CreatedEntityCarriesIdTagAndTransform)
{
	Entity entity = m_scene->CreateEntity("Player");

	ASSERT_TRUE(entity);
	EXPECT_TRUE(entity.HasComponent<Uge::IDComponent>());
	EXPECT_TRUE(entity.HasComponent<Uge::TagComponent>());
	EXPECT_TRUE(entity.HasComponent<Uge::TransformComponent>());
	EXPECT_EQ(entity.GetName(), "Player");
}

TEST_F(SceneTest, EntityWithoutANameGetsTheDefaultTag)
{
	Entity entity = m_scene->CreateEntity();

	EXPECT_FALSE(entity.GetName().empty());
}

TEST_F(SceneTest, UUIDsAreUniqueAcrossEntities)
{
	constexpr int count = 64;

	std::unordered_set<uint64_t> ids;
	for (int i = 0; i < count; i++)
	{
		ids.insert((uint64_t)m_scene->CreateEntity("Entity").GetUUID());
	}

	EXPECT_EQ(ids.size(), (size_t)count);
}

TEST_F(SceneTest, EntityCanBeLookedUpByUUIDAndByName)
{
	Entity entity = m_scene->CreateEntity("Camera");
	const Uge::UUID id = entity.GetUUID();

	EXPECT_EQ(m_scene->GetEntityByUUID(id), entity);
	EXPECT_EQ(m_scene->FindEntityByName("Camera"), entity);
}

TEST_F(SceneTest, LookupOfAMissingEntityReturnsAnInvalidHandle)
{
	EXPECT_FALSE(m_scene->FindEntityByName("does-not-exist"));
	EXPECT_FALSE(m_scene->GetEntityByUUID(Uge::UUID(1234567890)));
}

TEST_F(SceneTest, CreateEntityWithUUIDKeepsTheGivenId)
{
	// Loading a scene creates entities with their saved UUIDs.
	const Uge::UUID id(0xABCDEF12);
	Entity entity = m_scene->CreateEntityWithUUID(id, "Restored");

	EXPECT_EQ((uint64_t)entity.GetUUID(), (uint64_t)id);
	EXPECT_EQ(m_scene->GetEntityByUUID(id), entity);
}

TEST_F(SceneTest, DestroyEntityRemovesItFromTheSceneAndTheUUIDMap)
{
	Entity entity = m_scene->CreateEntity("Doomed");
	const Uge::UUID id = entity.GetUUID();

	m_scene->DestroyEntity(entity);

	EXPECT_EQ(m_scene->GetAllEntitiesWith<Uge::IDComponent>().size(), 0u);
	EXPECT_FALSE(m_scene->GetEntityByUUID(id));
}

TEST_F(SceneTest, DuplicateCopiesComponentsButNotTheIdentity)
{
	Entity source = m_scene->CreateEntity("Crate");
	source.GetComponent<Uge::TransformComponent>().Translation = glm::vec3(1.0f, 2.0f, 3.0f);
	source.AddComponent<Uge::SpriteRendererComponent>().Color = glm::vec4(0.25f, 0.5f, 0.75f, 1.0f);

	Entity copy = m_scene->DuplicateEntity(source);

	ASSERT_TRUE(copy);
	EXPECT_NE(copy, source);
	EXPECT_NE((uint64_t)copy.GetUUID(), (uint64_t)source.GetUUID());

	EXPECT_EQ(copy.GetName(), source.GetName());
	EXPECT_EQ(copy.GetComponent<Uge::TransformComponent>().Translation, glm::vec3(1.0f, 2.0f, 3.0f));
	ASSERT_TRUE(copy.HasComponent<Uge::SpriteRendererComponent>());
	EXPECT_EQ(copy.GetComponent<Uge::SpriteRendererComponent>().Color, glm::vec4(0.25f, 0.5f, 0.75f, 1.0f));
}

TEST_F(SceneTest, DuplicatedEntityIsIndependentOfItsSource)
{
	Entity source = m_scene->CreateEntity("Crate");
	Entity copy = m_scene->DuplicateEntity(source);

	copy.GetComponent<Uge::TransformComponent>().Translation = glm::vec3(9.0f);

	EXPECT_EQ(source.GetComponent<Uge::TransformComponent>().Translation, glm::vec3(0.0f));
}

TEST_F(SceneTest, PrimaryCameraEntityIsTheOneFlaggedPrimary)
{
	m_scene->CreateEntity("NotACamera");

	Entity secondary = m_scene->CreateEntity("Secondary");
	secondary.AddComponent<Uge::CameraComponent>().Primary = false;

	Entity primary = m_scene->CreateEntity("Primary");
	primary.AddComponent<Uge::CameraComponent>().Primary = true;

	EXPECT_EQ(m_scene->GetPrimaryCameraEntity(), primary);
}

TEST_F(SceneTest, NoPrimaryCameraYieldsAnInvalidEntity)
{
	m_scene->CreateEntity("NotACamera");

	EXPECT_FALSE(m_scene->GetPrimaryCameraEntity());
}

TEST_F(SceneTest, ACopiedSceneHasNoLiveBodies)
{
	// The copy has no body, as it builds its own physics world. The rest of the rigidbody is
	// copied.
	Entity entity = m_scene->CreateEntity("Crate");
	Uge::RigidbodyComponent& body = entity.AddComponent<Uge::RigidbodyComponent>();
	body.Mass = 3.0f;
	body.RuntimeBody.Value = 42;

	Uge::Ref<Scene> copy = Scene::Copy(m_scene);

	Entity copied = copy->GetEntityByUUID(entity.GetUUID());
	ASSERT_TRUE(copied);
	ASSERT_TRUE(copied.HasComponent<Uge::RigidbodyComponent>());

	const Uge::RigidbodyComponent& result = copied.GetComponent<Uge::RigidbodyComponent>();
	EXPECT_NEAR(result.Mass, 3.0f, 1e-5f);
	EXPECT_FALSE(result.RuntimeBody.IsValid());
}

namespace
{

	/** @brief A scene that can run Play: physics, and the script engine without Mono. */
	class ScenePlayTest : public UgeTests::PhysicsFixture
	{
	protected:
		void SetUp() override
		{
			PhysicsFixture::SetUp();
			if (std::filesystem::exists("mono/lib"))
			{
				GTEST_SKIP() << "Mono's runtime is in the working directory; run the tests from the repo root.";
			}
			Uge::ScriptEngine::Init();
		}

		void TearDown() override
		{
			Uge::ScriptEngine::Shutdown();
			PhysicsFixture::TearDown();
		}

		// The bodies within half a unit of the origin.
		std::vector<Uge::PhysicsBodyID> BodiesAtOrigin(Scene& scene)
		{
			std::vector<Uge::PhysicsBodyID> hits;
			scene.GetPhysicsScene()->OverlapSphere(glm::vec3(0.0f), 0.5f, Uge::PhysicsLayerMaskAll, hits);
			return hits;
		}
	};

}

TEST_F(ScenePlayTest, DestroyingAnEntityDuringPlayRemovesItsBody)
{
	Scene scene;
	Entity box = scene.CreateEntity("Box");
	box.AddComponent<Uge::RigidbodyComponent>();
	box.AddComponent<Uge::BoxColliderComponent>();

	scene.OnRuntimeStart();
	ASSERT_EQ(BodiesAtOrigin(scene).size(), 1u);

	scene.DestroyEntity(box);
	EXPECT_TRUE(BodiesAtOrigin(scene).empty());

	scene.OnRuntimeStop();
}

TEST_F(ScenePlayTest, RemovingARigidbodyDuringPlayRemovesItsBody)
{
	Scene scene;
	Entity box = scene.CreateEntity("Box");
	box.AddComponent<Uge::RigidbodyComponent>();
	box.AddComponent<Uge::BoxColliderComponent>();

	scene.OnRuntimeStart();
	ASSERT_EQ(BodiesAtOrigin(scene).size(), 1u);

	box.RemoveComponent<Uge::RigidbodyComponent>();
	EXPECT_TRUE(BodiesAtOrigin(scene).empty());

	scene.OnRuntimeStop();
}