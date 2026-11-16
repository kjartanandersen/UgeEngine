/**
 * @file Renderer2D.h
 * @brief The batched 2D renderer.
 * @ingroup group_renderer
 */

#pragma once

#include "Uge/Renderer/OrthographicCamera.h"
#include "Uge/Renderer/Camera.h"
#include "Uge/Renderer/Texture.h"
#include "Uge/Renderer/SubTexture2D.h"
#include "Uge/Renderer/EditorCamera.h"	
#include "Uge/Renderer/Font.h"	


#include "Uge/Scene/Components.h"

namespace Uge
{

	/**
	 * @brief Draws quads, sprites, text and lines in batches.
	 * @ingroup group_renderer
	 *
	 * A batch is drawn when it fills, when its texture slots run out, and at EndScene().
	 * Rotations are in radians. Its state is separate from Uge::Model's.
	 */
	class Renderer2D
	{

		
	public:
		/**
		 * @brief Creates the batch buffers, shaders and white texture.
		 */
		static void Init();
		/** @brief Releases the buffers, shaders and textures. */
		static void Shutdown();


		/**
		 * @brief Uploads the camera and starts a batch.
		 * @param camera The camera.
		 * @param transform The camera's world transform.
		 */
		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		/**
		 * @brief Uploads the editor camera and starts a batch.
		 * @param camera The camera.
		 */
		static void BeginScene(const EditorCamera& camera);
		/**
		 * @brief Restarts the quad batch without uploading the camera.
		 * @param camera Unused.
		 * @deprecated Used by the Sandbox; use the Uge::Camera overload.
		 */
		static void BeginScene(const OrthographicCamera& camera); // TODO: Remove
		/** @brief Draws what is batched. */
		static void EndScene();
		/** @brief Draws what is batched, without starting a new batch. */
		static void Flush();



		// Primitives
		/**
		 * @brief Draws a unit quad in a colour.
		 * @param transform Model matrix.
		 * @param color RGBA colour.
		 * @param entityID The entity, for mouse picking; `-1` for none.
		 */
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		/**
		 * @brief Draws a textured unit quad.
		 * @param transform Model matrix.
		 * @param texture The texture.
		 * @param tintColor Multiplied with the texture.
		 * @param entityID The entity, for mouse picking; `-1` for none.
		 */
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);

		/**
		 * @brief Draws a quad in a colour.
		 * @param position Centre; z is `0`.
		 * @param size Width and height.
		 * @param color RGBA colour.
		 */
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		/**
		 * @brief Draws a quad in a colour.
		 * @param position Centre; z orders it against other quads.
		 * @param size Width and height.
		 * @param color RGBA colour.
		 */
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		/**
		 * @brief Draws a textured quad.
		 * @param position Centre; z is `0`.
		 * @param size Width and height.
		 * @param texture The texture.
		 * @param tintColor Multiplied with the texture.
		 */
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));
		/**
		 * @brief Draws a textured quad.
		 * @param position Centre; z orders it against other quads.
		 * @param size Width and height.
		 * @param texture The texture.
		 * @param tintColor Multiplied with the texture.
		 */
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));
		/**
		 * @brief Draws a quad with part of a texture atlas.
		 * @param position Centre; z is `0`.
		 * @param size Width and height.
		 * @param subTexture The atlas region.
		 * @param tintColor Multiplied with the texture.
		 */
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor = glm::vec4(1.0f));
		/**
		 * @brief Draws a quad with part of a texture atlas.
		 * @param position Centre; z orders it against other quads.
		 * @param size Width and height.
		 * @param subTexture The atlas region.
		 * @param tintColor Multiplied with the texture.
		 */
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor = glm::vec4(1.0f));

		/**
		 * @brief Draws a rotated quad in a colour.
		 * @param position Centre; z is `0`.
		 * @param rotation Rotation about z, in radians.
		 * @param size Width and height.
		 * @param color RGBA colour.
		 */
		static void DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const glm::vec4& color);
		/**
		 * @brief Draws a rotated quad in a colour.
		 * @param position Centre; z orders it against other quads.
		 * @param rotation Rotation about z, in radians.
		 * @param size Width and height.
		 * @param color RGBA colour.
		 */
		static void DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const glm::vec4& color);
		/**
		 * @brief Draws a rotated textured quad.
		 * @param position Centre; z is `0`.
		 * @param rotation Rotation about z, in radians.
		 * @param size Width and height.
		 * @param texture The texture.
		 * @param tintColor Multiplied with the texture.
		 */
		static void DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));
		/**
		 * @brief Draws a rotated textured quad.
		 * @param position Centre; z orders it against other quads.
		 * @param rotation Rotation about z, in radians.
		 * @param size Width and height.
		 * @param texture The texture.
		 * @param tintColor Multiplied with the texture.
		 */
		static void DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));
		/**
		 * @brief Draws a rotated quad with part of a texture atlas.
		 * @param position Centre; z is `0`.
		 * @param rotation Rotation about z, in radians.
		 * @param size Width and height.
		 * @param subTexture The atlas region.
		 * @param tintColor Multiplied with the texture.
		 */
		static void DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor = glm::vec4(1.0f));
		/**
		 * @brief Draws a rotated quad with part of a texture atlas.
		 * @param position Centre; z orders it against other quads.
		 * @param rotation Rotation about z, in radians.
		 * @param size Width and height.
		 * @param subTexture The atlas region.
		 * @param tintColor Multiplied with the texture.
		 */
		static void DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor = glm::vec4(1.0f));

		/**
		 * @brief Draws a sprite. Without a texture that resolves, it is drawn in its colour.
		 * @param transform World transform.
		 * @param src The sprite.
		 * @param entityID The entity, for mouse picking; `-1` for none.
		 */
		static void DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& src, int entityID = -1);

		/** @brief Text colour and spacing. */
		struct TextParams
		{
			glm::vec4 Color{ 1.0f }; ///< RGBA colour.
			float Kerning = 0.0f; ///< Extra space between glyphs.
			float LineSpacing = 0.0f; ///< Extra space between lines.


		};
		/**
		 * @brief Draws a string.
		 * @param string UTF-8 text.
		 * @param font The font.
		 * @param transform Model matrix; the baseline starts at the origin.
		 * @param textParams Colour and spacing.
		 * @param entityID The entity, for mouse picking; `-1` for none.
		 */
		static void DrawString(const std::string& string, Ref<Font> font, const glm::mat4& transform, const TextParams& textParams, int entityID = -1);
		/**
		 * @brief Draws a string with a text component's font, colour and spacing.
		 * @param string UTF-8 text.
		 * @param transform World transform.
		 * @param component The text component.
		 * @param entityID The entity, for mouse picking; `-1` for none.
		 */
		static void DrawString(const std::string& string, const glm::mat4& transform, const TextComponent& component, int entityID = -1);
		
		/**
		 * @brief Draws a line, in a batch of its own.
		 * @param p0 Start, in world space.
		 * @param p1 End, in world space.
		 * @param color RGBA colour.
		 * @param entityID The entity, for mouse picking; `-1` for none.
		 */
		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, int entityID = -1);

		/**
		 * @brief Sets the line width, which applies to a whole batch.
		 * @param width Width in pixels. Most drivers clamp it to 1.
		 */
		static void SetLineWidth(float width);

		/**
		 * @brief The line width.
		 * @return Width in pixels.
		 */
		static float GetLineWidth();

	private:
		static void StartBatch();
		static void NextBatch();

		/**
		 * @brief A texture's slot in the batch. Adds the texture, drawing the batch first if
		 * the slots are full.
		 * @param texture The texture.
		 * @return The slot, as a float.
		 */
		static float GetTextureSlot(const Ref<Texture2D>& texture);


	};



}
