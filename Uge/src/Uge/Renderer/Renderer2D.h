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
	



	class Renderer2D
	{

		
	public:
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() const { return QuadCount * 6; }


		};

	public:

		static void Init();
		static void Shutdown();


		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(const OrthographicCamera& camera); // TODO: Remove
		static void EndScene();
		static void Flush();



		// Primitives
		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);

		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor = glm::vec4(1.0f));

		// Rotations is in radians
		static void DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor = glm::vec4(1.0f));

		static void DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& src, int entityID = -1);

		static void DrawString(const std::string& string, Ref<Font> font, const glm::mat4& transform, const glm::vec4& color);
		

	private:
		static void FlushAndReset();
		static void StartBatch();


	};



}
