/**
 * @file Renderer.h
 * @brief Renderer lifecycle and the simple forward submission path.
 * @ingroup group_renderer
 */

#pragma once

#include "RenderCommand.h"
#include "Camera.h"
#include "Shader.h"

namespace Uge
{
	/**
	 * @brief Owns renderer start-up and shutdown, and a minimal shader submission path.
	 * @ingroup group_renderer
	 *
	 * Init() and Shutdown() are the important part: Uge::Application calls them, and they
	 * bring up every other draw path, including Uge::Renderer2D.
	 *
	 * The BeginScene()/Submit()/EndScene() trio here is the original unbatched forward
	 * path, which only uploads a view-projection matrix and a transform. Prefer
	 * Uge::Renderer2D for sprites and Uge::Model for meshes.
	 *
	 * @warning This scene state is separate from Uge::Renderer2D's and Uge::Model's. Do not
	 * interleave their begin/end pairs.
	 */
	class Renderer
	{
	public:

		/**
		 * @brief Initializes the graphics API and every draw path.
		 *
		 * Called by the Uge::Application constructor, after the window and its context exist.
		 */
		static void Init();
		/** @brief Releases renderer-wide resources. Called from ~Application. */
		static void Shutdown();
		/**
		 * @brief Resizes the backbuffer viewport in response to a window resize.
		 * @param width New width in pixels.
		 * @param height New height in pixels.
		 */
		static void OnWindowResize(uint32_t width, uint32_t height);

		/**
		 * @brief Begins a frame, capturing the camera's view-projection matrix.
		 * @param camera Camera supplying the view-projection matrix.
		 */
		static void BeginScene(Camera& camera);
		/** @brief Ends the frame opened by BeginScene(). */
		static void EndScene();

		/**
		 * @brief Draws geometry with a shader and a model transform.
		 * @param shader Shader to bind; receives the view-projection and transform uniforms.
		 * @param vertexArray Geometry to draw.
		 * @param transform Model matrix; defaults to identity.
		 *
		 * Issues one draw call per submission — there is no batching or sorting here.
		 */
		static void Submit(const Ref<Shader>& shader,
			const Ref<VertexArray> vertexArray, const glm::mat4& transform = glm::mat4(1.0f));


		/**
		 * @brief Returns the active graphics API.
		 * @return The Uge::RendererAPI::API in use.
		 */
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

	private:
		/** @brief Per-frame state captured by BeginScene(). */
		struct SceneData
		{

			glm::mat4 viewProjectionMatrix;


		};

		static SceneData* m_sceneData;

	};



}


