/**
 * @file Renderer.h
 * @brief Renderer startup and shutdown.
 * @ingroup group_renderer
 */

#pragma once

#include "RenderCommand.h"
#include "Camera.h"
#include "Shader.h"

namespace Uge
{
	/**
	 * @brief Starts and stops the renderers, and has an unbatched draw path.
	 * @ingroup group_renderer
	 *
	 * Use Uge::Renderer2D and Uge::Model rather than Submit().
	 */
	class Renderer
	{
	public:

		/**
		 * @brief Sets the initial GPU state and starts Uge::Renderer2D. Needs a GL context.
		 *
		 * The other renderers create their resources on first use.
		 */
		static void Init();
		/** @brief Releases the GPU objects the renderers keep in statics, while the GL context exists. */
		static void Shutdown();
		/**
		 * @brief Sets the viewport to the window's size.
		 * @param width Width in pixels.
		 * @param height Height in pixels.
		 */
		static void OnWindowResize(uint32_t width, uint32_t height);

		/**
		 * @brief Does nothing; the camera isn't stored.
		 * @param camera The camera.
		 */
		static void BeginScene(Camera& camera);
		/** @brief Does nothing. */
		static void EndScene();

		/**
		 * @brief Draws geometry in one draw call.
		 * @param shader The shader, given `u_ViewProjection` and `u_ModelMatrix`.
		 * @param vertexArray The geometry.
		 * @param transform Model matrix.
		 */
		static void Submit(const Ref<Shader>& shader,
			const Ref<VertexArray> vertexArray, const glm::mat4& transform = glm::mat4(1.0f));


		/**
		 * @brief The graphics API in use.
		 * @return The API.
		 */
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:
		/** @brief The camera matrix Submit() uploads. */
		struct SceneData
		{

			glm::mat4 viewProjectionMatrix;


		};

		static SceneData* m_sceneData;

	};



}


