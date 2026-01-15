#include "ugpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include "glm/glm.hpp"
#include <glm/ext/matrix_transform.hpp>

namespace Uge
{

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
	};

	struct Renderer2DData
	{
		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> QuadVA;
		Ref<VertexBuffer> QuadVB;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 is white texture

		glm::vec4 QuadVertexPositions[4];


	};

	static Renderer2DData m_data;




	void Renderer2D::Init()
	{
		UG_PROFILE_FUNCTION();
		m_data.QuadVA = VertexArray::Create();

		m_data.QuadVB = VertexBuffer::Create(m_data.MaxVertices * sizeof(QuadVertex));
		BufferLayout squareVBlayout =
		{
			{ ShaderDataType::Float3, "a_Position"},
			{ ShaderDataType::Float4, "a_Color"},
			{ ShaderDataType::Float2, "a_TextCoord"},
			{ ShaderDataType::Float, "a_TexIndex"},
			{ ShaderDataType::Float, "a_TilingFactor"}
		};
		m_data.QuadVB->SetLayout(squareVBlayout);
		m_data.QuadVA->AddVertexBuffer(m_data.QuadVB);

		m_data.QuadVertexBufferBase = new QuadVertex[m_data.MaxVertices];


		uint32_t* quadIndices = new uint32_t[m_data.MaxIndices];
		uint32_t offset = 0;
		for (int i = 0; i < m_data.MaxIndices; i += 6)
		{

			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;

		}
		Ref<IndexBuffer> squareIB = IndexBuffer::Create(quadIndices, m_data.MaxIndices);
		m_data.QuadVA->SetIndexBuffer(squareIB);
		delete[] quadIndices;

		m_data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		m_data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		int32_t samplers[m_data.MaxTextureSlots];
		for (uint32_t i = 0; i < m_data.MaxTextureSlots; i++)
		{
			samplers[i] = i;
		}


		m_data.TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		m_data.TextureShader->Bind();
		m_data.TextureShader->SetIntArray("u_Textures", samplers, m_data.MaxTextureSlots);


		// Set all texture slots to zero
		
		m_data.TextureSlots[0] = m_data.WhiteTexture;

		m_data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		m_data.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		m_data.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		m_data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		


	}

	void Renderer2D::Shutdown()
	{
		UG_PROFILE_FUNCTION();

		

	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		UG_PROFILE_FUNCTION();

		m_data.TextureShader->Bind();
		m_data.TextureShader->SetMat4(
			"u_ViewProjection", camera.GetViewProjectionMatrix());

		m_data.QuadVertexBufferPtr = m_data.QuadVertexBufferBase;
		m_data.QuadIndexCount = 0;

		m_data.TextureSlotIndex = 1;
		


	}

	void Renderer2D::EndScene()
	{
		UG_PROFILE_FUNCTION();

		uint32_t dataSize = (uint8_t*)m_data.QuadVertexBufferPtr - (uint8_t*)m_data.QuadVertexBufferBase;
		m_data.QuadVB->SetData(m_data.QuadVertexBufferBase, dataSize);

		Flush();

	}

	void Renderer2D::Flush()
	{

		// Bind textures
		for (uint32_t i = 0; i < m_data.TextureSlotIndex; i++)
			m_data.TextureSlots[i]->Bind(i);

		

		RenderCommand::DrawIndexed(m_data.QuadVA, m_data.QuadIndexCount);


	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{

		DrawQuad({ position.x, position.y, 0.0f }, size, color );


	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		UG_PROFILE_FUNCTION();

		const float texIndex = 0.0f; // White texture
		const float tilingFactor = 1.0f; // Tiling Factor

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[0];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		m_data.QuadVertexBufferPtr->TexIndex = texIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[1];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		m_data.QuadVertexBufferPtr->TexIndex = texIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[2];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		m_data.QuadVertexBufferPtr->TexIndex = texIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[3];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		m_data.QuadVertexBufferPtr->TexIndex = texIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;



		m_data.QuadIndexCount += 6;

		/*
		m_data.TextureShader->SetFloat("u_TilingFactor", 1.0f);
		m_data.WhiteTexture->Bind();
		glm::mat4 transform = 
			glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		m_data.TextureShader->SetMat4("u_ModelMatrix", transform);
		m_data.SquareVA->Bind();
		RenderCommand::DrawIndexed(m_data.SquareVA);
		*/



	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{

		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);

	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		UG_PROFILE_FUNCTION();

		constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

		float textureIndex = 0.0f;

		for (uint32_t i = 0; i < m_data.TextureSlotIndex; i++)
		{

			if (*m_data.TextureSlots[i].get() == *texture.get())
			{

				textureIndex = (float)i;
				break;

			}

		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)m_data.TextureSlotIndex;
			m_data.TextureSlots[m_data.TextureSlotIndex] = texture;
			m_data.TextureSlotIndex++;

		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[0];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		m_data.QuadVertexBufferPtr->TexIndex = textureIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[1];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		m_data.QuadVertexBufferPtr->TexIndex = textureIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[2];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		m_data.QuadVertexBufferPtr->TexIndex = textureIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[3];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		m_data.QuadVertexBufferPtr->TexIndex = textureIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		m_data.QuadIndexCount += 6;


		/*m_data.TextureShader->SetFloat4("u_Color", tintColor);
		m_data.TextureShader->SetFloat("u_TilingFactor", tilingFactor);
		texture->Bind();

		

		glm::mat4 transform = 
			glm::translate(glm::mat4(1.0f), position) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 0.1f });

		m_data.TextureShader->SetMat4("u_ModelMatrix", transform);

		

		m_data.QuadVA->Bind();
		RenderCommand::DrawIndexed(m_data.QuadVA);
		texture->UnBind();
		*/

	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, rotation, size, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const glm::vec4& color)
	{

		UG_PROFILE_FUNCTION();

		const float texIndex = 0.0f; // White texture
		const float tilingFactor = 1.0f; // Tiling Factor

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0, 0, 1 })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[0];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		m_data.QuadVertexBufferPtr->TexIndex = texIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[1];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		m_data.QuadVertexBufferPtr->TexIndex = texIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[2];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		m_data.QuadVertexBufferPtr->TexIndex = texIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[3];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		m_data.QuadVertexBufferPtr->TexIndex = texIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;



		m_data.QuadIndexCount += 6;

	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, rotation, size, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{

		UG_PROFILE_FUNCTION();

		constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

		float textureIndex = 0.0f;

		for (uint32_t i = 0; i < m_data.TextureSlotIndex; i++)
		{

			if (*m_data.TextureSlots[i].get() == *texture.get())
			{

				textureIndex = (float)i;
				break;

			}

		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)m_data.TextureSlotIndex;
			m_data.TextureSlots[m_data.TextureSlotIndex] = texture;
			m_data.TextureSlotIndex++;

		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0, 0, 1 })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[0];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		m_data.QuadVertexBufferPtr->TexIndex = textureIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[1];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		m_data.QuadVertexBufferPtr->TexIndex = textureIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[2];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		m_data.QuadVertexBufferPtr->TexIndex = textureIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		m_data.QuadVertexBufferPtr->Position = transform * m_data.QuadVertexPositions[3];
		m_data.QuadVertexBufferPtr->Color = color;
		m_data.QuadVertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		m_data.QuadVertexBufferPtr->TexIndex = textureIndex;
		m_data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
		m_data.QuadVertexBufferPtr++;

		

		m_data.QuadIndexCount += 6;

	}

}