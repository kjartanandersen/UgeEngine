#include "ugpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"
#include "UniformBuffer.h"
#include "Mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>




namespace Uge
{

	struct Renderer2DData
	{
		static const uint32_t MaxQuads = 10000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> QuadVA;
		Ref<VertexBuffer> QuadVB;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		Vertex* VertexBufferBase = nullptr;
		Vertex* VertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 is white texture

		glm::vec4 VertexPositions[4];
		

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};

		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;


	};

	static Renderer2DData m_data;




	void Renderer2D::Init()
	{
		UG_PROFILE_FUNCTION();
		m_data.QuadVA = VertexArray::Create();

		m_data.QuadVB = VertexBuffer::Create(m_data.MaxVertices * sizeof(Vertex));
		BufferLayout squareVBlayout =
		{
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float2, "a_TextCoord"    },
			{ ShaderDataType::Float,  "a_TexIndex"     },
			{ ShaderDataType::Float,  "a_TilingFactor" },
			{ ShaderDataType::Int,    "a_EntityID"     }
		};
		m_data.QuadVB->SetLayout(squareVBlayout);
		m_data.QuadVA->AddVertexBuffer(m_data.QuadVB);

		m_data.VertexBufferBase = new Vertex[m_data.MaxVertices];


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

		// m_data.WhiteTexture = Texture2D::Create(1, 1);
		m_data.WhiteTexture = Texture2D::Create(TextureSpecification());
		uint32_t whiteTextureData = 0xffffffff;
		m_data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		int32_t samplers[m_data.MaxTextureSlots];
		for (uint32_t i = 0; i < m_data.MaxTextureSlots; i++)
		{
			samplers[i] = i;
		}


		m_data.TextureShader = Shader::Create("assets/shaders/Texture.glsl");


		// Set all texture slots to zero
		
		m_data.TextureSlots[0] = m_data.WhiteTexture;

		m_data.VertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		m_data.VertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		m_data.VertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		m_data.VertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		m_data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);


	}

	void Renderer2D::Shutdown()
	{
		UG_PROFILE_FUNCTION();

		

	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		UG_PROFILE_FUNCTION();

		m_data.TextureShader->Bind();
		//m_data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

		m_data.VertexBufferPtr = m_data.VertexBufferBase;
		m_data.QuadIndexCount = 0;

		m_data.TextureSlotIndex = 1;

		m_data.QuadVB->Bind();
		m_data.QuadVA->Bind();
		


	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{

		UG_PROFILE_FUNCTION();

		m_data.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		m_data.CameraUniformBuffer->SetData(&m_data.CameraBuffer, sizeof(Renderer2DData::CameraData));

		m_data.QuadVB->Bind();
		m_data.QuadVA->Bind();

		StartBatch();



	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{

		UG_PROFILE_FUNCTION();

		m_data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		m_data.CameraUniformBuffer->SetData(&m_data.CameraBuffer, sizeof(Renderer2DData::CameraData));

		m_data.QuadVB->Bind();
		m_data.QuadVA->Bind();

		StartBatch();


	}

	void Renderer2D::EndScene()
	{
		UG_PROFILE_FUNCTION();

		uint32_t dataSize = (uint8_t*)m_data.VertexBufferPtr - (uint8_t*)m_data.VertexBufferBase;
		m_data.QuadVB->SetData(m_data.VertexBufferBase, dataSize);

		Flush();

	}

	void Renderer2D::Flush()
	{

		// Bind textures
		for (uint32_t i = 0; i < m_data.TextureSlotIndex; i++)
			m_data.TextureSlots[i]->Bind(i);

		
		m_data.TextureShader->Bind();
		if (m_data.QuadIndexCount != 0)
		{
			RenderCommand::DrawIndexed(m_data.QuadVA, m_data.QuadIndexCount);
		}

		m_data.TextureShader->Unbind();

		m_data.VertexBufferPtr = m_data.VertexBufferBase;
		m_data.QuadIndexCount = 0;

		m_data.TextureSlotIndex = 1;


	}

	void Uge::Renderer2D::FlushAndReset()
	{
		EndScene();

		m_data.VertexBufferPtr = m_data.VertexBufferBase;
		m_data.QuadIndexCount = 0;

		m_data.TextureSlotIndex = 1;

	}



	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{

		DrawQuad({ position.x, position.y, 0.0f }, size, color );


	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		UG_PROFILE_FUNCTION();


		
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, color);


	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor)
	{

		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tintColor);

	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor)
	{

		DrawQuad({ position.x, position.y, 0.0f }, size, subTexture, tintColor);


	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor)
	{

		UG_PROFILE_FUNCTION();

		constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		const glm::vec2* textureCoords = subTexture->GetTextCoords();
		const Ref<Texture2D> texture = subTexture->GetTexture();

		if (m_data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{

			FlushAndReset();

		}


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

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[0];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = textureCoords[0];
		m_data.VertexBufferPtr->TexIndex = textureIndex;
		m_data.VertexBufferPtr->TilingFactor = texture->m_tilingFactor;
		m_data.VertexBufferPtr++;

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[1];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = textureCoords[1];
		m_data.VertexBufferPtr->TexIndex = textureIndex;
		m_data.VertexBufferPtr->TilingFactor = texture->m_tilingFactor;
		m_data.VertexBufferPtr++;

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[2];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = textureCoords[2];
		m_data.VertexBufferPtr->TexIndex = textureIndex;
		m_data.VertexBufferPtr->TilingFactor = texture->m_tilingFactor;
		m_data.VertexBufferPtr++;

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[3];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = textureCoords[3];
		m_data.VertexBufferPtr->TexIndex = textureIndex;
		m_data.VertexBufferPtr->TilingFactor = texture->m_tilingFactor;
		m_data.VertexBufferPtr++;



		m_data.QuadIndexCount += 6;



	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID )
	{
		UG_PROFILE_FUNCTION();


		if (m_data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{

			FlushAndReset();

		}

		constexpr size_t VertexCount = 4;
		const float texIndex = 0.0f; // White texture
		const float tilingFactor = 1.0f; // Tiling Factor
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		

		

		for (size_t i = 0; i < VertexCount; i++)
		{

			m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[i];
			m_data.VertexBufferPtr->Color = color;
			m_data.VertexBufferPtr->TexCoord = textureCoords[i];
			m_data.VertexBufferPtr->TexIndex = texIndex;
			m_data.VertexBufferPtr->TilingFactor = tilingFactor;
			m_data.VertexBufferPtr->EntityID = entityID;
			m_data.VertexBufferPtr++;

		}



		m_data.QuadIndexCount += 6;



	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, const glm::vec4& tintColor, int entityID )
	{

		UG_PROFILE_FUNCTION();

		if (m_data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{

			FlushAndReset();

		}


		constexpr size_t VertexCount = 4;
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		float texIndex = 0.0f;


		for (uint32_t i = 0; i < m_data.TextureSlotIndex; i++)
		{

			if (*m_data.TextureSlots[i].get() == *texture.get())
			{

				texIndex = (float)i;
				break;

			}

		}

		if (texIndex == 0.0f)
		{
			texIndex = (float)m_data.TextureSlotIndex;
			m_data.TextureSlots[m_data.TextureSlotIndex] = texture;
			m_data.TextureSlotIndex++;

		}

		for (size_t i = 0; i < VertexCount; i++)
		{

			m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[i];
			m_data.VertexBufferPtr->Color = tintColor;
			m_data.VertexBufferPtr->TexCoord = textureCoords[i];
			m_data.VertexBufferPtr->TexIndex = texIndex;
			m_data.VertexBufferPtr->TilingFactor = texture->m_tilingFactor;
			m_data.VertexBufferPtr->EntityID = entityID;
			m_data.VertexBufferPtr++;

		}



		m_data.QuadIndexCount += 6;



	}


	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor)
	{
		UG_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, texture, tintColor);

	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, rotation, size, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const glm::vec4& color)
	{

		UG_PROFILE_FUNCTION();

		if (m_data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{

			FlushAndReset();

		}

		const float texIndex = 0.0f; // White texture
		const float tilingFactor = 1.0f; // Tiling Factor

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), rotation, { 0, 0, 1 })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[0];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		m_data.VertexBufferPtr->TexIndex = texIndex;
		m_data.VertexBufferPtr->TilingFactor = tilingFactor;
		m_data.VertexBufferPtr++;

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[1];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		m_data.VertexBufferPtr->TexIndex = texIndex;
		m_data.VertexBufferPtr->TilingFactor = tilingFactor;
		m_data.VertexBufferPtr++;

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[2];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		m_data.VertexBufferPtr->TexIndex = texIndex;
		m_data.VertexBufferPtr->TilingFactor = tilingFactor;
		m_data.VertexBufferPtr++;

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[3];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		m_data.VertexBufferPtr->TexIndex = texIndex;
		m_data.VertexBufferPtr->TilingFactor = tilingFactor;
		m_data.VertexBufferPtr++;



		m_data.QuadIndexCount += 6;


	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, rotation, size, texture, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture, const glm::vec4& tintColor)
	{

		UG_PROFILE_FUNCTION();

		if (m_data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{

			FlushAndReset();

		}

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
			* glm::rotate(glm::mat4(1.0f), rotation, { 0, 0, 1 })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[0];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = { 0.0f, 0.0f };
		m_data.VertexBufferPtr->TexIndex = textureIndex;
		m_data.VertexBufferPtr->TilingFactor = texture->m_tilingFactor;
		m_data.VertexBufferPtr++;

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[1];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = { 1.0f, 0.0f };
		m_data.VertexBufferPtr->TexIndex = textureIndex;
		m_data.VertexBufferPtr->TilingFactor = texture->m_tilingFactor;
		m_data.VertexBufferPtr++;

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[2];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = { 1.0f, 1.0f };
		m_data.VertexBufferPtr->TexIndex = textureIndex;
		m_data.VertexBufferPtr->TilingFactor = texture->m_tilingFactor;
		m_data.VertexBufferPtr++;

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[3];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = { 0.0f, 1.0f };
		m_data.VertexBufferPtr->TexIndex = textureIndex;
		m_data.VertexBufferPtr->TilingFactor = texture->m_tilingFactor;
		m_data.VertexBufferPtr++;

		

		m_data.QuadIndexCount += 6;


	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor)
	{

		DrawRotatedQuad({position.x, position.y, 0.0f}, rotation, size, subTexture, tintColor);

	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<SubTexture2D>& subTexture, const glm::vec4& tintColor)
	{

		UG_PROFILE_FUNCTION();

		constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		const glm::vec2* textureCoords = subTexture->GetTextCoords();
		const Ref<Texture2D> texture = subTexture->GetTexture();

		if (m_data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{

			FlushAndReset();

		}


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
			* glm::rotate(glm::mat4(1.0f), rotation, { 0, 0, 1 })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[0];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = textureCoords[0];
		m_data.VertexBufferPtr->TexIndex = textureIndex;
		m_data.VertexBufferPtr->TilingFactor = texture->m_tilingFactor;
		m_data.VertexBufferPtr++;

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[1];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = textureCoords[1];
		m_data.VertexBufferPtr->TexIndex = textureIndex;
		m_data.VertexBufferPtr->TilingFactor = texture->m_tilingFactor;
		m_data.VertexBufferPtr++;

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[2];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = textureCoords[2];
		m_data.VertexBufferPtr->TexIndex = textureIndex;
		m_data.VertexBufferPtr->TilingFactor = texture->m_tilingFactor;
		m_data.VertexBufferPtr++;

		m_data.VertexBufferPtr->Position = transform * m_data.VertexPositions[3];
		m_data.VertexBufferPtr->Color = color;
		m_data.VertexBufferPtr->TexCoord = textureCoords[3];
		m_data.VertexBufferPtr->TexIndex = textureIndex;
		m_data.VertexBufferPtr->TilingFactor = texture->m_tilingFactor;
		m_data.VertexBufferPtr++;



		m_data.QuadIndexCount += 6;


	}


	void Renderer2D::StartBatch()
	{

		m_data.QuadIndexCount = 0;
		m_data.VertexBufferPtr = m_data.VertexBufferBase;

		m_data.TextureSlotIndex = 1;

	}


	void Renderer2D::DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& src, int entityID)
	{

		UG_PROFILE_FUNCTION();

		if (src.Texture)
		{
			DrawQuad(transform, src.Texture, src.Color, entityID);

		}
		else
		{
			DrawQuad(transform, src.Color, entityID);

		}


	}

}