/**
 * @file Renderer2D.h
 * @brief The batched 2D draw path: quads, sprites and MSDF text.
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
	 * @brief Batched renderer for quads, sprites and text.
	 * @ingroup group_renderer
	 *
	 * Accumulates geometry into a large vertex buffer and issues a single draw call per
	 * batch, so drawing a thousand sprites costs roughly one draw call rather than a
	 * thousand. A batch is flushed automatically when the vertex buffer fills up or when
	 * more distinct textures are used than there are texture slots.
	 *
	 * Every draw takes an optional `entityID` written to the framebuffer's integer
	 * attachment, which is what lets the editor resolve a click to an entity.
	 *
	 * @code
	 * Renderer2D::BeginScene(camera, transform);
	 * Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 1.0f, 1.0f }, { 1, 0, 0, 1 });
	 * Renderer2D::DrawSprite(transform, spriteComponent, (int)entity);
	 * Renderer2D::EndScene();       // flushes the final batch
	 * @endcode
	 *
	 * @warning State is entirely separate from Uge::Renderer3D and Uge::Model. Do not
	 * interleave their begin/end pairs — close one before opening another.
	 *
	 * @note All rotation parameters are in **radians**.
	 */
	class Renderer2D
	{

		
	public:
		/**
		 * @brief Per-frame batching counters, shown in the editor's stats panel.
		 *
		 * Useful for spotting batch breaks: a quad count far below the draw call count means
		 * something is forcing a flush, usually too many distinct textures.
		 */
		struct Statistics
		{
			uint32_t DrawCalls = 0; ///< Draw calls issued this frame.
			uint32_t QuadCount = 0; ///< Quads submitted this frame.

			/** @brief Vertices submitted this frame. @return Four per quad. */
			uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
			/** @brief Indices submitted this frame. @return Six per quad. */
			uint32_t GetTotalIndexCount() const { return QuadCount * 6; }


		};

		

	public:

		/**
		 * @brief Allocates the batch buffers, shaders and the default white texture.
		 *
		 * Called by Uge::Renderer::Init; clients do not call it themselves.
		 */
		static void Init();
		/** @brief Releases the batching resources. */
		static void Shutdown();


		/**
		 * @brief Begins a batch using a scene camera and its entity transform.
		 * @param camera Camera supplying the projection.
		 * @param transform The camera entity's world transform; its inverse becomes the view.
		 *
		 * The runtime form, used when playing a scene through a Uge::CameraComponent.
		 */
		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		/**
		 * @brief Begins a batch using the editor's orbit camera.
		 * @param camera Editor camera supplying the view-projection matrix.
		 */
		static void BeginScene(const EditorCamera& camera);
		/**
		 * @brief Begins a batch using a standalone orthographic camera.
		 * @param camera Camera supplying the projection.
		 * @deprecated Kept for the Sandbox samples; prefer the Uge::Camera overload.
		 */
		static void BeginScene(const OrthographicCamera& camera); // TODO: Remove
		/** @brief Ends the batch and flushes any remaining geometry. */
		static void EndScene();
		/** @brief Issues a draw call for the geometry accumulated so far. */
		static void Flush();



		// Primitives
		/**
		 * @brief Draws a solid-colour quad from a full transform matrix.
		 * @param transform Model matrix positioning and sizing the unit quad.
		 * @param color RGBA tint, components in `[0, 1]`.
		 * @param entityID ID written to the picking attachment; `-1` for none.
		 */
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		/**
		 * @brief Draws a textured quad from a full transform matrix.
		 * @param transform Model matrix positioning and sizing the unit quad.
		 * @param texture Texture to sample.
		 * @param tintColor Multiplied with the sampled colour; white leaves it unchanged.
		 * @param entityID ID written to the picking attachment; `-1` for none.
		 */
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);

		/**
		 * @brief Draws a solid-colour quad at a 2D position.
		 * @param position Centre position; z is `0`.
		 * @param size Width and height in world units.
		 * @param color RGBA colour.
		 */
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		/**
		 * @brief Draws a solid-colour quad at a 3D position.
		 * @param position Centre position; z orders it against other quads.
		 * @param size Width and height in world units.
		 * @param color RGBA colour.
		 */
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		/**
		 * @brief Draws a textured quad at a 2D position.
		 * @param position Centre position; z is `0`.
		 * @param size Width and height in world units.
		 * @param texture Texture to sample.
		 * @param tintColor Multiplied with the sampled colour.
		 */
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));
		/**
		 * @brief Draws a textured quad at a 3D position.
		 * @param position Centre position; z orders it against other quads.
		 * @param size Width and height in world units.
		 * @param texture Texture to sample.
		 * @param tintColor Multiplied with the sampled colour.
		 */
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));
		/**
		 * @brief Draws an atlas region at a 2D position.
		 * @param position Centre position; z is `0`.
		 * @param size Width and height in world units.
		 * @param subTexture Atlas region supplying the texture coordinates.
		 * @param tintColor Multiplied with the sampled colour.
		 */
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor = glm::vec4(1.0f));
		/**
		 * @brief Draws an atlas region at a 3D position.
		 * @param position Centre position; z orders it against other quads.
		 * @param size Width and height in world units.
		 * @param subTexture Atlas region supplying the texture coordinates.
		 * @param tintColor Multiplied with the sampled colour.
		 */
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor = glm::vec4(1.0f));

		// Rotations is in radians
		/**
		 * @brief Draws a rotated solid-colour quad at a 2D position.
		 * @param position Centre position; z is `0`.
		 * @param rotation Rotation about the z axis, in **radians**.
		 * @param size Width and height in world units.
		 * @param color RGBA colour.
		 */
		static void DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const glm::vec4& color);
		/**
		 * @brief Draws a rotated solid-colour quad at a 3D position.
		 * @param position Centre position; z orders it against other quads.
		 * @param rotation Rotation about the z axis, in **radians**.
		 * @param size Width and height in world units.
		 * @param color RGBA colour.
		 */
		static void DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const glm::vec4& color);
		/**
		 * @brief Draws a rotated textured quad at a 2D position.
		 * @param position Centre position; z is `0`.
		 * @param rotation Rotation about the z axis, in **radians**.
		 * @param size Width and height in world units.
		 * @param texture Texture to sample.
		 * @param tintColor Multiplied with the sampled colour.
		 */
		static void DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));
		/**
		 * @brief Draws a rotated textured quad at a 3D position.
		 * @param position Centre position; z orders it against other quads.
		 * @param rotation Rotation about the z axis, in **radians**.
		 * @param size Width and height in world units.
		 * @param texture Texture to sample.
		 * @param tintColor Multiplied with the sampled colour.
		 */
		static void DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));
		/**
		 * @brief Draws a rotated atlas region at a 2D position.
		 * @param position Centre position; z is `0`.
		 * @param rotation Rotation about the z axis, in **radians**.
		 * @param size Width and height in world units.
		 * @param subTexture Atlas region supplying the texture coordinates.
		 * @param tintColor Multiplied with the sampled colour.
		 */
		static void DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor = glm::vec4(1.0f));
		/**
		 * @brief Draws a rotated atlas region at a 3D position.
		 * @param position Centre position; z orders it against other quads.
		 * @param rotation Rotation about the z axis, in **radians**.
		 * @param size Width and height in world units.
		 * @param subTexture Atlas region supplying the texture coordinates.
		 * @param tintColor Multiplied with the sampled colour.
		 */
		static void DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor = glm::vec4(1.0f));

		/**
		 * @brief Draws an entity's sprite component.
		 * @param transform The entity's world transform.
		 * @param src Sprite component supplying the texture, tint and tiling factor.
		 * @param entityID ID written to the picking attachment; `-1` for none.
		 *
		 * The bridge between the ECS and this renderer: Uge::Scene calls it for every entity
		 * holding a Uge::SpriteRendererComponent.
		 */
		static void DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& src, int entityID = -1);

		/** @brief Colour and spacing options for a text draw. */
		struct TextParams
		{
			glm::vec4 Color{ 1.0f }; ///< Text colour, RGBA.
			float Kerning = 0.0f; ///< Extra spacing between glyphs, in world units.
			float LineSpacing = 0.0f; ///< Extra spacing between lines, in world units.


		};
		/**
		 * @brief Draws a string using a multi-channel signed distance field font.
		 * @param string Text to draw; interpreted as UTF-8.
		 * @param font Font supplying the glyph atlas and metrics.
		 * @param transform Model matrix positioning the text; the baseline starts at the origin.
		 * @param textParams Colour, kerning and line spacing.
		 * @param entityID ID written to the picking attachment; `-1` for none.
		 *
		 * MSDF glyphs stay sharp at any scale, unlike a conventional bitmap font atlas.
		 */
		static void DrawString(const std::string& string, Ref<Font> font, const glm::mat4& transform, const TextParams& textParams, int entityID = -1);
		/**
		 * @brief Draws an entity's text component.
		 * @param string Text to draw.
		 * @param transform The entity's world transform.
		 * @param component Text component supplying the font, colour and spacing.
		 * @param entityID ID written to the picking attachment; `-1` for none.
		 */
		static void DrawString(const std::string& string, const glm::mat4& transform, const TextComponent& component, int entityID = -1);
		

	private:
		static void FlushAndReset();
		static void StartBatch();


	};



}
