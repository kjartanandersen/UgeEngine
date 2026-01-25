#pragma once

#include "Uge.h"

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

		OrthographicCameraController m_cameraController;

		// TODO: Temp
		Ref<Shader> m_flatColorShader;
		Ref<VertexArray> m_squareVA;

		Ref<Texture2D> m_texture;
		Ref<Texture2D> m_spriteSheet;
		Ref<SubTexture2D> m_textureStairs, m_textureBarrel, m_textureDirt;

		Ref<Framebuffer> m_frameBuffer;

		Ref<Scene> m_activeScene;
		entt::entity m_squareEnt;

		ImFont* m_mainFont;

		glm::vec4 m_square1Color = { 1.0f, 0.1f, 0.1f, 1.0f };
		glm::vec4 m_square2Color = { 0.1f, 0.1f, 1.1f, 1.0f };

		glm::vec2 m_viewportSize{0.0f, 0.0f};

		bool m_shouldResize = false;
		bool m_viewportFocused = false;
		bool m_viewportHovered = false;

		uint32_t m_mapWidth, m_mapHeight;
		std::unordered_map<char, Ref<SubTexture2D>> m_textureMap;


	};


}

