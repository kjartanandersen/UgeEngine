/**
 * @file Renderer3D.h
 * @brief Standalone textured-cube draw path.
 * @ingroup group_renderer
 */

#pragma once

#include "PerspectiveCamera.h"
#include "Texture.h"

namespace Uge
{

	/**
	 * @brief Draws textured cube primitives.
	 * @ingroup group_renderer
	 *
	 * The narrowest of the three draw paths: cubes only, unbatched, with a fixed shader.
	 * It is useful for quick visualization and for the Sandbox samples.
	 *
	 * @note For actual 3D content use Uge::Model, which handles imported meshes,
	 * materials and the scene's mesh components.
	 *
	 * @warning Keeps its own scene state, independent of Uge::Renderer2D and Uge::Model.
	 * Do not interleave their begin/end pairs.
	 */
	class Renderer3D
	{

	public:
		/** @brief Allocates the cube geometry and shader. */
		static void Init();
		/** @brief Releases the cube resources. */
		static void Shutdown();


		/**
		 * @brief Begins a frame with the given camera.
		 * @param camera Camera supplying the view-projection matrix.
		 */
		static void BeginScene(const PerspectiveCamera& camera);
		/** @brief Ends the frame opened by BeginScene(). */
		static void EndScene();

		// Primitives
		/**
		 * @brief Draws a solid-colour cube.
		 * @param position Centre position in world space.
		 * @param rotation Rotation about the y axis, in radians.
		 * @param size Extents along each axis, in world units.
		 * @param color RGBA colour.
		 */
		static void DrawCube(const glm::vec3& position, float rotation, const glm::vec3& size, const glm::vec4& color);
		/**
		 * @brief Draws a textured cube.
		 * @param position Centre position in world space.
		 * @param rotation Rotation about the y axis, in radians.
		 * @param size Extents along each axis, in world units.
		 * @param texture Texture applied to every face.
		 * @param tilingFactor How many times the texture repeats across a face.
		 * @param tintColor Multiplied with the sampled colour.
		 */
		static void DrawCube(const glm::vec3& position, float rotation, const glm::vec3& size, Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor = glm::vec4(1.0f));




	};


}
