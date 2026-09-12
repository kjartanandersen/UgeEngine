/**
 * @file SceneTests.cpp
 * @brief Tests for the entity operations the editor's hierarchy panel drives.
 */

#include <ugpch.h>

#include "Uge/Scene/Components.h"
#include "Uge/Scene/Entity.h"
#include "Uge/Scene/Scene.h"

#include <gtest/gtest.h>

#include <unordered_set>

using Uge::Entity;
using Uge::Scene;

namespace
{

	/** @brief Gives each test an empty scene. */
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
	// Deserialization takes this path: the id in the .uge file has to survive, or every
	// entity reference in the scene breaks on load.
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
