/**
 * @file EditorLayer.h
 * @brief The layer implementing essentially all of the editor's behaviour.
 * @ingroup group_editor
 */

#pragma once

#include "Uge.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"

namespace Uge
{

	/**
	 * @brief The editor: viewport, gizmos, mouse picking, play controls and scene commands.
	 * @ingroup group_editor
	 *
	 * Renders the scene into an off-screen Uge::Framebuffer and displays its colour
	 * attachment as an ImGui image, which is what makes the viewport a dockable panel.
	 * A second `RED_INTEGER` attachment stores a per-pixel entity ID, read back on click
	 * to resolve the entity under the cursor.
	 *
	 * Play mode runs on a **copy** of the scene, so entering and leaving it leaves the
	 * edited scene untouched:
	 *
	 * @code
	 * m_editorScene = m_activeScene;              // keep the edited scene
	 * m_activeScene = Scene::Copy(m_editorScene); // play on a copy
	 * m_activeScene->OnRuntimeStart();
	 * @endcode
	 *
	 * @see Uge::SceneHierarchyPanel, Uge::ContentBrowserPanel
	 */
	class EditorLayer : public Layer
	{

	public:
		/** @brief Constructs the layer; resources are created in OnAttach(). */
		EditorLayer();
		/** @brief Default destructor. */
		virtual ~EditorLayer() = default;

		/** @brief Creates the framebuffer, loads editor icons and opens the startup project. */
		virtual void OnAttach() override;
		/** @brief Releases the editor's resources. */
		virtual void OnDetach() override;

		/**
		 * @brief Resizes the framebuffer if needed, renders the scene and reads back the
		 *        hovered entity.
		 * @param ts Frame delta time.
		 *
		 * Dispatches to Uge::Scene::OnUpdateEditor or Uge::Scene::OnUpdateRuntime depending on
		 * the current scene state.
		 */
		virtual void OnUpdate(Timestep ts) override;
		/**
		 * @brief Forwards events to the editor camera and handles editor shortcuts.
		 * @param e Event to handle.
		 */
		virtual void OnEvent(Event& e) override;
		/**
		 * @brief Emits the dockspace, menu bar, viewport, gizmos, stats and every panel.
		 */
		virtual void OnImGuiRender() override;

	private:
		/**
		 * @brief Handles editor keyboard shortcuts.
		 * @param e The key event.
		 * @return `true` if the shortcut was consumed.
		 *
		 * Covers new/open/save, entity duplication and delete, and the Q/W/E/R gizmo modes.
		 */
		bool OnKeyPressed(KeyPressedEvent& e);
		/**
		 * @brief Selects the entity under the cursor on a left click in the viewport.
		 * @param e The mouse event.
		 * @return `true` if the click was consumed.
		 *
		 * Ignored while a gizmo is being manipulated or the Alt camera modifier is held.
		 */
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		/** @brief Writes the active project and its asset registry to disk. */
		void SaveProject();
		/** @brief Creates a new empty project and makes it active. */
		void NewProject();
		/**
		 * @brief Prompts for a `.ugproj` file and opens it.
		 * @return `true` if a project was opened; `false` if the dialog was cancelled.
		 */
		bool OpenProject();
		/**
		 * @brief Opens a project and loads its start scene.
		 * @param path Path to the `.ugproj` file.
		 */
		void OpenProject(const std::filesystem::path& path);

		/** @brief Prompts for a path and saves the current scene to it. */
		void SaveSceneAs();
		/** @brief Saves the current scene to its existing path, or prompts if it has none. */
		void SaveScene();
		/** @brief Replaces the current scene with an empty one. */
		void NewScene();
		/** @brief Prompts for a `.uge` file and opens it as the current scene. */
		void OpenScene();
		/**
		 * @brief Opens a scene asset by handle.
		 * @param handle Handle of the scene to open.
		 * @note Stops play mode first if it is running.
		 */
		void OpenScene(AssetHandle handle);

		/**
		 * @brief Enters play mode on a copy of the edited scene.
		 *
		 * The edited scene is kept aside so OnSceneStop() can restore it unchanged.
		 */
		void OnScenePlay();
		/** @brief Leaves play mode and restores the edited scene. */
		void OnSceneStop();
		/** @brief Toggles the paused state of the running scene. */
		void OnScenePause();

		/**
		 * @brief Writes a scene to a path via Uge::SceneSerializer.
		 * @param scene Scene to save.
		 * @param path Destination `.uge` path.
		 */
		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		// UI panels
		/** @brief Draws the play/pause/step/stop toolbar above the viewport. */
		void UI_Toolbar();

		/** @brief Duplicates the selected entity and selects the copy. */
		void OnDuplicateEntry();


	private:


		// TODO: Temp
		Ref<Shader> m_flatColorShader;
		Ref<VertexArray> m_squareVA;

		Ref<Texture2D> m_texture;
		Ref<Texture2D> m_spriteSheet;

		Ref<Framebuffer> m_frameBuffer;

		Ref<Scene> m_activeScene;
		Ref<Scene> m_editorScene;
		std::filesystem::path m_editorScenePath;

		EditorCamera m_editorCamera;

		ImFont* m_mainFont;
		ImFont* m_mainFontBold;

		glm::vec2 m_viewportSize{0.0f, 0.0f};
		glm::vec2 m_viewportBounds[2];

		// Panels
		SceneHierarchyPanel m_sceneHierarchyPanel;
		Scope<ContentBrowserPanel> m_contentBrowserPanel;

		Entity m_hoveredEntity;

		int m_gizmoType = -1;

		bool m_shouldResize = false;
		bool m_viewportFocused = false;
		bool m_viewportHovered = false;

		// Editor Resources
		/** @brief Whether the editor is editing or playing the scene. */
		enum class SceneState
		{
			Edit = 0, ///< Editing; the scene renders from the editor camera.
			Play = 1 ///< Playing; scripts run and the scene renders from its primary camera.
		};

		SceneState m_sceneState = SceneState::Edit;

		Ref<Texture2D> m_iconPlay, m_iconStop, m_iconPause, m_iconStep;



	};


}

