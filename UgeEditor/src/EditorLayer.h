/**
 * @file EditorLayer.h
 * @brief The editor layer.
 * @ingroup group_editor
 */

#pragma once

#include "Uge.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/RendererSettingsPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/DebugPanel.h"

namespace Uge
{

	/**
	 * @brief The editor: viewport, gizmos, mouse picking, play mode, menus and panels.
	 * @ingroup group_editor
	 *
	 * Play mode runs on a copy of the edited scene.
	 */
	class EditorLayer : public Layer
	{

	public:
		/** @brief Does nothing; resources are created in OnAttach(). */
		EditorLayer();
		/** @brief Default destructor. */
		virtual ~EditorLayer() = default;

		/**
		 * @brief Opens the project named on the command line, or asks for one and closes the
		 * program if none is picked. Then creates the framebuffers and loads the icons and fonts.
		 */
		virtual void OnAttach() override;
		/** @brief Stops play mode if it is running. */
		virtual void OnDetach() override;

		/**
		 * @brief Renders the scene, finds the hovered entity, and runs bloom and tonemapping.
		 * @param ts Frame time.
		 */
		virtual void OnUpdate(Timestep ts) override;
		/**
		 * @brief Passes events to the editor camera and handles shortcuts and clicks.
		 * @param e The event.
		 */
		virtual void OnEvent(Event& e) override;
		/**
		 * @brief Draws the dockspace, menus, viewport, gizmo, toolbar and panels.
		 */
		virtual void OnImGuiRender() override;

	private:
		/**
		 * @brief Handles shortcuts, except while the editor camera is turning.
		 *
		 * Ctrl+N, Ctrl+O, Ctrl+S and Ctrl+Shift+S for scenes, Ctrl+P to open a project,
		 * Ctrl+D to duplicate, Delete, Ctrl+R to reload scripts, and Q, W, E, R and T for the
		 * gizmo.
		 *
		 * @param e The key event.
		 * @return `false`, so the event goes on to other handlers.
		 */
		bool OnKeyPressed(KeyPressedEvent& e);
		/**
		 * @brief Selects the hovered entity on a left click in the viewport, unless the cursor
		 * is over the gizmo or Alt is held.
		 * @param e The mouse event.
		 * @return `false`, so the event goes on to other handlers.
		 */
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		/** @brief Does nothing; its body is commented out. */
		void SaveProject();
		/** @brief Makes a new, unsaved project active. */
		void NewProject();
		/**
		 * @brief Asks for a `.ugproj` file and opens it.
		 * @return false if the dialog was cancelled or the project didn't load.
		 */
		bool OpenProject();
		/**
		 * @brief Opens a project, starts scripting, and opens its start scene if it has one.
		 * @param path The `.ugproj` file.
		 * @return false if the project didn't load; the open project then stays active.
		 */
		bool OpenProject(const std::filesystem::path& path);

		/**
		 * @brief Asks for a path and saves the scene there. Registers it if it is a `.uge` file
		 * in the asset directory. Refused in play mode.
		 */
		void SaveSceneAs();
		/** @brief Saves the scene to its path, or calls SaveSceneAs() if it has none. Refused in play mode. */
		void SaveScene();
		/** @brief Stops play mode and opens an empty scene. */
		void NewScene();
		/** @brief Does nothing; its body is commented out. */
		void OpenScene();
		/**
		 * @brief Stops play mode and opens a copy of a scene asset, reloaded from disk.
		 * @param handle The scene's handle.
		 */
		void OpenScene(AssetHandle handle);

		/**
		 * @brief Starts play mode on a copy of the edited scene.
		 */
		void OnScenePlay();
		/** @brief Stops play mode and goes back to the edited scene. */
		void OnSceneStop();
		/** @brief Pauses the running scene. Does nothing in Edit mode. */
		void OnScenePause();

		/**
		 * @brief Saves a scene.
		 * @param scene The scene.
		 * @param path The `.uge` file.
		 * @return `true` if it was written; otherwise the serializer has logged why.
		 */
		bool SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		// UI panels
		/** @brief Draws the play, pause, step and stop toolbar. */
		void UI_Toolbar();

		/** @brief Duplicates the selected entity and selects the copy. Edit mode only. */
		void OnDuplicateEntry();

		/**
		 * @brief Draws the collider wireframes from the collider components, if View >
		 * Physics Colliders is on.
		 *
		 * Uses the editor camera in Edit mode and the primary camera in Play. Runs after the
		 * picking readback, so the lines don't affect picking.
		 */
		void OnOverlayRender();


	private:


		// TODO: Temp
		Ref<Shader> m_flatColorShader;
		Ref<VertexArray> m_squareVA;

		Ref<Texture2D> m_texture;
		Ref<Texture2D> m_spriteSheet;

		Ref<Framebuffer> m_frameBuffer;
		Ref<Framebuffer> m_displayFrameBuffer;

		Ref<Scene> m_activeScene;
		Ref<Scene> m_editorScene;
		std::filesystem::path m_editorScenePath;

		EditorCamera m_editorCamera;

		glm::vec2 m_viewportSize{0.0f, 0.0f};
		glm::vec2 m_viewportBounds[2];

		// Panels
		SceneHierarchyPanel m_sceneHierarchyPanel;
		Scope<ContentBrowserPanel> m_contentBrowserPanel;
		RendererSettingsPanel m_rendererSettingsPanel;
		ConsolePanel m_consolePanel;
		DebugPanel m_debugPanel;

		// Panel visibility, driven by the View menu.
		bool m_showSceneHierarchy = true;
		bool m_showContentBrowser = true;
		bool m_showRendererSettings = true;
		bool m_showConsole = true;
		bool m_showDebug = true;
		bool m_showLoadedAssets = false;

		// Collider overlay, set from the View menu.
		bool m_showPhysicsColliders = false; ///< Whether collider wireframes are drawn.
		bool m_colliderXRay = true; ///< Whether they are drawn without the depth test.

		Entity m_hoveredEntity;

		int m_gizmoType = -1;

		bool m_shouldResize = false;
		bool m_viewportFocused = false;
		bool m_viewportHovered = false;

		// Editor Resources
		/** @brief Whether the scene is being edited or played. */
		enum class SceneState
		{
			Edit = 0, ///< Rendered from the editor camera.
			Play = 1 ///< Running, and rendered from the primary camera.
		};

		SceneState m_sceneState = SceneState::Edit;

		Ref<Texture2D> m_iconPlay, m_iconStop, m_iconPause, m_iconStep;

		std::unordered_map<AssetHandle, std::vector<glm::vec3>> m_colliderEdgeCache;

	};


}

