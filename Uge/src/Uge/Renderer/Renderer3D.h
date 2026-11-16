/**
 * @file Renderer3D.h
 * @brief A cube renderer.
 * @ingroup group_renderer
 */

#pragma once

#include "PerspectiveCamera.h"
#include "Texture.h"

namespace Uge
{

	/**
	 * @brief Draws cubes, one draw call each. Used by the Sandbox.
	 * @ingroup group_renderer
	 *
	 * Init()'s body is commented out, so there is no shader or geometry to draw with.
	 */
	class Renderer3D
	{

	public:
		/** @brief Does nothing; its body is commented out. */
		static void Init();
		/** @brief Does nothing. */
		static void Shutdown();


		/**
		 * @brief Binds the shader. The camera isn't uploaded.
		 * @param camera The camera.
		 */
		static void BeginScene(const PerspectiveCamera& camera);
		/** @brief Does nothing. */
		static void EndScene();

		// Primitives
		/**
		 * @brief Draws a cube with the checkerboard texture, tinted.
		 * @param position Centre, in world space.
		 * @param rotation Rotation about z, in degrees.
		 * @param size Size along each axis.
		 * @param color RGBA tint.
		 */
		static void DrawCube(const glm::vec3& position, float rotation, const glm::vec3& size, const glm::vec4& color);
		/**
		 * @brief Draws a textured cube.
		 * @param position Centre, in world space.
		 * @param rotation Rotation about z, in degrees.
		 * @param size Size along each axis.
		 * @param texture The texture, on every face.
		 * @param tilingFactor How many times the texture repeats on a face.
		 * @param tintColor Multiplied with the texture.
		 */
		static void DrawCube(const glm::vec3& position, float rotation, const glm::vec3& size, Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor = glm::vec4(1.0f));




	};


}
