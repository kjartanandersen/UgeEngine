/**
 * @file SceneHierarchyPanelTests.cpp
 * @brief Tests for the editor's hierarchy and properties panel, drawn headlessly.
 */

#include <ugpch.h>

#include "Panels/SceneHierarchyPanel.h"

#include "TestSupport/HeadlessImGui.h"
#include "TestSupport/MockTexture2D.h"
#include "TestSupport/TempProject.h"

#include "Uge/Asset/EditorAssetManager.h"
#include "Uge/Project/Project.h"
#include "Uge/Scene/Components.h"
#include "Uge/Scene/Entity.h"
#include "Uge/Scene/Scene.h"

#include <gtest/gtest.h>

using Uge::Entity;
using Uge::Project;
using Uge::Scene;
using Uge::SceneHierarchyPanel;
using UgeTests::TempProject;

namespace
{

	/**
	 * @brief An ImGui context and an active project, which the panel needs both of.
	 *
	 * Every asset-backed component the properties inspector draws asks
	 * Uge::AssetManager whether its handle is valid, and that facade asserts when no
	 * project is loaded — so the panel cannot be drawn without one.
	 */
	class SceneHierarchyPanelTest : public UgeTests::HeadlessImGuiTest
	{
	protected:
		void SetUp() override
		{
			UgeTests::HeadlessImGuiTest::SetUp();
			m_scene = Uge::CreateRef<Scene>();
		}

		Uge::Ref<Uge::EditorAssetManager> Manager() const
		{
			return Project::GetActive()->GetEditorAssetManager();
		}

		TempProject m_project;
		SceneHierarchyPanel m_panel;
		Uge::Ref<Scene> m_scene;
	};

}

TEST_F(SceneHierarchyPanelTest, RendersWithNoContext)
{
	// The editor draws its panels before a scene is opened.
	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(SceneHierarchyPanelTest, RendersAnEmptyScene)
{
	m_panel.SetContext(m_scene);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(SceneHierarchyPanelTest, RendersATreeOfEntities)
{
	for (int i = 0; i < 10; i++)
	{
		m_scene->CreateEntity("Entity" + std::to_string(i));
	}

	m_panel.SetContext(m_scene);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(SceneHierarchyPanelTest, SetContextClearsTheSelection)
{
	Entity entity = m_scene->CreateEntity("Selected");
	m_panel.SetContext(m_scene);
	m_panel.SetSelectedEntity(entity);
	ASSERT_TRUE(m_panel.GetSelectedEntity());

	// Opening another scene must drop the handle: it points into the old registry, and
	// dereferencing it once that scene is gone is undefined.
	m_panel.SetContext(Uge::CreateRef<Scene>());

	EXPECT_FALSE(m_panel.GetSelectedEntity());
}

TEST_F(SceneHierarchyPanelTest, RendersThePropertiesOfASelectedEntity)
{
	Entity entity = m_scene->CreateEntity("Selected");

	m_panel.SetContext(m_scene);
	m_panel.SetSelectedEntity(entity);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });

	EXPECT_EQ(m_panel.GetSelectedEntity(), entity);
}

TEST_F(SceneHierarchyPanelTest, RendersEveryComponentInspector)
{
	// One entity carrying everything: each component has its own DrawComponent block with
	// its own ImGui id scope and style pushes, and this is what walks all of them.
	Entity entity = m_scene->CreateEntity("Everything");
	entity.AddComponent<Uge::CameraComponent>();
	entity.AddComponent<Uge::SpriteRendererComponent>();
	entity.AddComponent<Uge::MeshComponent>();
	entity.AddComponent<Uge::SkyLightComponent>();
	entity.AddComponent<Uge::DirectionalLightComponent>();
	entity.AddComponent<Uge::TextComponent>();

	m_panel.SetContext(m_scene);
	m_panel.SetSelectedEntity(entity);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(SceneHierarchyPanelTest, RendersAnOrthographicCameraInspector)
{
	// The projection dropdown draws a different set of widgets per type.
	Entity entity = m_scene->CreateEntity("Camera");
	entity.AddComponent<Uge::CameraComponent>().Cam.SetProjectionType(
		Uge::SceneCamera::ProjectionType::Orthographic);

	m_panel.SetContext(m_scene);
	m_panel.SetSelectedEntity(entity);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(SceneHierarchyPanelTest, RendersAPerspectiveCameraInspector)
{
	Entity entity = m_scene->CreateEntity("Camera");
	entity.AddComponent<Uge::CameraComponent>().Cam.SetProjectionType(
		Uge::SceneCamera::ProjectionType::Perspective);

	m_panel.SetContext(m_scene);
	m_panel.SetSelectedEntity(entity);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(SceneHierarchyPanelTest, RendersAnUnassignedAssetSlotAsNone)
{
	Entity entity = m_scene->CreateEntity("Sprite");
	entity.AddComponent<Uge::SpriteRendererComponent>().Texture = 0;

	m_panel.SetContext(m_scene);
	m_panel.SetSelectedEntity(entity);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(SceneHierarchyPanelTest, RendersADanglingAssetHandleWithoutResolvingIt)
{
	// A scene saved against content that has since been deleted still has to open, showing
	// the slot as invalid rather than asserting on the missing asset.
	Entity entity = m_scene->CreateEntity("Broken");
	entity.AddComponent<Uge::SpriteRendererComponent>().Texture = Uge::AssetHandle(0xDEAD);
	entity.AddComponent<Uge::MeshComponent>().Mesh = Uge::AssetHandle(0xBEEF);
	entity.AddComponent<Uge::SkyLightComponent>().Environment = Uge::AssetHandle(0xF00D);

	m_panel.SetContext(m_scene);
	m_panel.SetSelectedEntity(entity);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });

	// Drawing must not have registered anything to make those handles resolve.
	EXPECT_FALSE(Manager()->IsAssetHandleValid(Uge::AssetHandle(0xDEAD)));
}

TEST_F(SceneHierarchyPanelTest, RendersAnAssignedTextureSlot)
{
	// The valid-asset branch reads the registry rather than loading the texture, which is
	// what makes a mock enough to reach it without a GPU.
	Uge::Ref<UgeTests::MockTexture2D> texture = Uge::CreateRef<UgeTests::MockTexture2D>();
	const Uge::AssetHandle handle = Manager()->AddMemoryOnlyAsset(texture);

	Entity entity = m_scene->CreateEntity("Sprite");
	entity.AddComponent<Uge::SpriteRendererComponent>().Texture = handle;

	m_panel.SetContext(m_scene);
	m_panel.SetSelectedEntity(entity);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });

	EXPECT_EQ(entity.GetComponent<Uge::SpriteRendererComponent>().Texture, handle);
}

TEST_F(SceneHierarchyPanelTest, RendersEntitiesWithAwkwardNames)
{
	// Tags are user text and reach ImGui as labels; one used as a format string, or one
	// that collides in the id stack, shows up here.
	m_scene->CreateEntity("100% of %s");
	m_scene->CreateEntity("##hidden");
	m_scene->CreateEntity("");
	m_scene->CreateEntity("Duplicate");
	m_scene->CreateEntity("Duplicate");

	m_panel.SetContext(m_scene);

	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}

TEST_F(SceneHierarchyPanelTest, RendersAfterTheSelectedEntityIsDestroyed)
{
	Entity entity = m_scene->CreateEntity("Doomed");
	m_panel.SetContext(m_scene);
	m_panel.SetSelectedEntity(entity);

	m_scene->DestroyEntity(entity);

	// The panel's selection is now a stale handle; the frame has to notice it is no longer
	// valid instead of reading components off it.
	DrawFrames(2, [this]() { m_panel.OnImGuiRender(); });
}
