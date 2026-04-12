#pragma once

#include "Uge.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"

namespace Uge
{

	class EditorLayer : public Layer
	{

	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnEvent(Event& e) override;
		virtual void OnImGuiRender() override;

	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void SaveScene();
		void SaveSceneAs();
		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneStop();
		void OnScenePause();

		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		// UI panels
		void UI_Toolbar();

	private:


		// TODO: Temp
		Ref<Shader> m_flatColorShader;
		Ref<VertexArray> m_squareVA;

		Ref<Texture2D> m_texture;
		Ref<Texture2D> m_spriteSheet;
		Ref<SubTexture2D> m_textureStairs, m_textureBarrel, m_textureDirt;

		Ref<Framebuffer> m_frameBuffer;

		Ref<Scene> m_activeScene;
		Ref<Scene> m_editorScene;
		std::filesystem::path m_editorScenePath;

		EditorCamera m_editorCamera;

		ImFont* m_mainFont;
		ImFont* m_mainFontBold;

		glm::vec4 m_square1Color = { 1.0f, 0.1f, 0.1f, 1.0f };
		glm::vec4 m_square2Color = { 0.1f, 0.1f, 1.1f, 1.0f };

		glm::vec2 m_viewportSize{0.0f, 0.0f};
		glm::vec2 m_viewportBounds[2];

		// Panels
		SceneHierarchyPanel m_sceneHierarchyPanel;
		ContentBrowserPanel m_contentBrowserPanel;

		Entity m_hoveredEntity;

		int m_gizmoType = -1;

		bool m_shouldResize = false;
		bool m_viewportFocused = false;
		bool m_viewportHovered = false;

		// Editor Resources
		enum class SceneState
		{
			Edit = 0, 
			Play = 1
		};

		SceneState m_sceneState = SceneState::Edit;

		Ref<Texture2D> m_iconPlay, m_iconStop, m_iconPause, m_iconStep;



	};


}

