#include "ugpch.h"
#include "Renderer2D.h"

#include "Uge/Renderer/VertexArray.h"
#include "Uge/Renderer/Shader.h"
#include "Uge/Renderer/RenderCommand.h"
#include "Uge/Renderer/UniformBuffer.h"
#include "Uge/Renderer/Mesh.h"
#include "Uge/Renderer/MSDFData.h"

#include "Uge/Asset/AssetManager.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>




namespace Uge
{

	/** @brief Vertex format for MSDF text quads. */
	struct TextVertex
	{
		glm::vec3 Position; ///< Glyph corner position in world space.
		glm::vec4 Color; ///< Text colour, RGBA.
		glm::vec2 TexCoord; ///< Coordinates into the MSDF atlas.

		// TODO: bg color for outline/bg

		// Editor Only
		int EntityID; ///< Owning entity, written to the picking attachment. Editor only.
	};

	/**
	 * @brief All batching state for Uge::Renderer2D: buffers, shaders and texture slots.
	 *
	 * A single file-static instance. The vertex buffers are CPU-side staging arrays that
	 * are filled as primitives are submitted and uploaded once per flush.
	 */
	struct Renderer2DData
	{
		static const uint32_t MaxQuads = 10000; ///< Maximum quads per batch before an automatic flush.
		static const uint32_t MaxVertices = MaxQuads * 4; ///< Vertex capacity implied by #MaxQuads.
		static const uint32_t MaxIndices = MaxQuads * 6; ///< Index capacity implied by #MaxQuads.
		static const uint32_t MaxTextureSlots = 32; ///< Texture units available; exceeding this forces a flush.

		Ref<VertexArray> QuadVA; ///< Vertex array for the quad batch.
		Ref<VertexBuffer> QuadVB; ///< Dynamic vertex buffer refilled each flush.
		Ref<Shader> TextureShader; ///< Shader used for quads and sprites.
		Ref<Texture2D> WhiteTexture; ///< 1x1 white texture, bound to slot 0 for untextured quads.

		Ref<VertexArray> TextVA; ///< Vertex array for the text batch.
		Ref<VertexBuffer> TextVB; ///< Dynamic vertex buffer for text geometry.
		Ref<Shader> TextShader; ///< Shader that samples the MSDF atlas.

		uint32_t QuadIndexCount = 0; ///< Indices accumulated in the current quad batch.
		Vertex* VertexBufferBase = nullptr; ///< Start of the CPU-side quad staging buffer.
		Vertex* VertexBufferPtr = nullptr; ///< Write cursor into the quad staging buffer.

		uint32_t TextIndexCount = 0; ///< Indices accumulated in the current text batch.
		TextVertex* TextVertexBufferBase = nullptr; ///< Start of the CPU-side text staging buffer.
		TextVertex* TextVertexBufferPtr = nullptr; ///< Write cursor into the text staging buffer.

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots; ///< Textures bound for the current batch.
		uint32_t TextureSlotIndex = 1; ///< Next free texture slot; slot 0 is #WhiteTexture.

		Ref<Texture2D> FontAtlasTexture; ///< Atlas of the font used by the current text batch.

		glm::vec4 VertexPositions[4] = { {} }; ///< Unit-quad corners, transformed per draw.
		

		/** @brief Camera block uploaded to the shared uniform buffer once per scene. */
		struct CameraData
		{
			glm::mat4 ViewProjection; ///< Combined view-projection matrix.
		};

		CameraData CameraBuffer = { {} }; ///< CPU-side copy of the camera block.
		Ref<UniformBuffer> CameraUniformBuffer; ///< GPU uniform buffer holding #CameraBuffer.


	};

	static Renderer2DData m_data;


	void Renderer2D::Init()
	{
		UG_PROFILE_FUNCTION();
		
		// Quad
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

		// Text
		m_data.TextVA = VertexArray::Create();


		m_data.TextVB = VertexBuffer::Create(m_data.MaxVertices * sizeof(Vertex));
		BufferLayout textVBlayout =
		{
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float2, "a_TextCoord"    },
			{ ShaderDataType::Int,    "a_EntityID"     }
		};
		m_data.TextVB->SetLayout(textVBlayout);
		m_data.TextVA->AddVertexBuffer(m_data.TextVB);

		m_data.TextVertexBufferBase = new TextVertex[m_data.MaxVertices];


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
		m_data.TextVA->SetIndexBuffer(squareIB);
		delete[] quadIndices;

		// m_data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		TextureSpecification spec = TextureSpecification();


		m_data.WhiteTexture = Texture2D::Create(spec, Buffer(&whiteTextureData, sizeof(uint32_t)));
		m_data.WhiteTexture->SetName("WhiteTexture");
		// &whiteTextureData, sizeof(uint32_t)

		int32_t samplers[m_data.MaxTextureSlots];
		for (uint32_t i = 0; i < m_data.MaxTextureSlots; i++)
		{
			samplers[i] = i;
		}


		m_data.TextureShader = Shader::Create("assets/shaders/Texture.glsl");
		m_data.TextShader    = Shader::Create("assets/shaders/Text.glsl");


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

		if (m_data.QuadIndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)m_data.VertexBufferPtr - (uint8_t*)m_data.VertexBufferBase);
			m_data.QuadVB->SetData(m_data.VertexBufferBase, dataSize);

			// Bind textures
			for (uint32_t i = 0; i < m_data.TextureSlotIndex; i++)
			{
				m_data.TextureSlots[i]->Bind(i);
			}


			m_data.TextureShader->Bind();
			RenderCommand::DrawIndexed(m_data.QuadVA, m_data.QuadIndexCount);

			RenderStats::Get().Quad2DCount += m_data.QuadIndexCount / 6;
		}

		if (m_data.TextIndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)m_data.TextVertexBufferPtr - (uint8_t*)m_data.TextVertexBufferBase);
			m_data.TextVB->SetData(m_data.TextVertexBufferBase, dataSize);

			m_data.FontAtlasTexture->Bind(0);


			m_data.TextShader->Bind();
			RenderCommand::DrawIndexed(m_data.TextVA, m_data.TextIndexCount);

			RenderStats::Get().Text2DQuadCount += m_data.TextIndexCount / 6;
		}

		/*
		m_data.TextureShader->Unbind();

		m_data.VertexBufferPtr = m_data.VertexBufferBase;
		m_data.QuadIndexCount = 0;

		m_data.TextureSlotIndex = 1;
		*/

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
		UG_CORE_VERIFY(texture);

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

		m_data.TextIndexCount = 0;
		m_data.TextVertexBufferPtr = m_data.TextVertexBufferBase;

		m_data.TextureSlotIndex = 1;

	}

	void Renderer2D::DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& src, int entityID)
	{

		UG_PROFILE_FUNCTION();

		if (src.Texture)
		{
			Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(src.Texture);
			DrawQuad(transform, texture, src.Color, entityID);

		}
		else
		{
			DrawQuad(transform, src.Color, entityID);

		}


	}

	void Renderer2D::DrawString(const std::string& string, Ref<Font> font, const glm::mat4& transform, const TextParams& textParams, int entityID)
	{

		const auto& fontGeometry = font->GetMSDFData()->Fonts;
		const auto& metrics = fontGeometry.getMetrics();
		Ref<Texture2D> fontAtlas = font->GetAtlasTexture();
		

		m_data.FontAtlasTexture = fontAtlas;
		
		double x = 0.0;
		double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
		double y = 0.0;		

		const float spaceGlyphAdvance = fontGeometry.getGlyph(' ')->getAdvance();

		for (size_t i = 0; i < string.size(); i++)
		{

			char character = string[i];
			if (character == '\r')
			{
				continue;
			}

			if (character == '\n')
			{
				x = 0;
				y -= fsScale * metrics.lineHeight + textParams.LineSpacing;
				continue;
			}
			


			auto glyph = fontGeometry.getGlyph(character);
			if (!glyph)
			{
				glyph = fontGeometry.getGlyph('?');
			}
			if (!glyph)
			{
				return;
			}
			


			if (character == ' ')
			{
				float advance = spaceGlyphAdvance;
				if (i < string.size() - 1)
				{
					char nextCharacter = string[i + 1];
					double dAdvance;
					fontGeometry.getAdvance(dAdvance, character, nextCharacter);
					advance = (float)dAdvance;


				}
				x += fsScale * advance + textParams.Kerning;
				continue;

			}

			if (character == '\t')
			{
				x += (fsScale * spaceGlyphAdvance + textParams.Kerning) * 4.0;
				continue;
			}

			double al, ab, ar, at;
			glyph->getQuadAtlasBounds(al, ab, ar, at);
			glm::vec2 texCoordMin((float)al, (float)ab);
			glm::vec2 texCoordMax((float)ar, (float)at);

			double pl, pb, pr, pt;
			glyph->getQuadPlaneBounds(pl, pb, pr, pt);
			glm::vec2 quadMin((float)pl, (float)pb);
			glm::vec2 quadMax((float)pr, (float)pt);

			quadMin *= fsScale, quadMax *= fsScale;
			quadMin += glm::vec2(x, y);
			quadMax += glm::vec2(x, y);

			float texelWidth = 1.0f / fontAtlas->GetWidth();
			float texelHeight = 1.0f / fontAtlas->GetHeight();
			texCoordMin *= glm::vec2(texelWidth, texelHeight);
			texCoordMax *= glm::vec2(texelWidth, texelHeight);

			// render here
			m_data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin, 0.0f, 1.0f);
			m_data.TextVertexBufferPtr->Color = textParams.Color;
			m_data.TextVertexBufferPtr->TexCoord = texCoordMin;
			m_data.TextVertexBufferPtr->EntityID = entityID;
			m_data.TextVertexBufferPtr++;

			m_data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
			m_data.TextVertexBufferPtr->Color = textParams.Color;
			m_data.TextVertexBufferPtr->TexCoord = { texCoordMin.x, texCoordMax.y };
			m_data.TextVertexBufferPtr->EntityID = entityID;
			m_data.TextVertexBufferPtr++;

			m_data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax, 0.0f, 1.0f);
			m_data.TextVertexBufferPtr->Color = textParams.Color;
			m_data.TextVertexBufferPtr->TexCoord = texCoordMax;
			m_data.TextVertexBufferPtr->EntityID = entityID;
			m_data.TextVertexBufferPtr++;

			m_data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
			m_data.TextVertexBufferPtr->Color = textParams.Color;
			m_data.TextVertexBufferPtr->TexCoord = { texCoordMax.x, texCoordMin.y };
			m_data.TextVertexBufferPtr->EntityID = entityID;
			m_data.TextVertexBufferPtr++;

			m_data.TextIndexCount += 6;

			if (i < string.size() - 1)
			{
				double advance = glyph->getAdvance();

				char nextCharacter = string[i + 1];
				fontGeometry.getAdvance(advance, character, nextCharacter);

				x += fsScale * advance + textParams.Kerning;
			}



		}


	}

	void Renderer2D::DrawString(const std::string& string, const glm::mat4& transform, const TextComponent& component, int entityID)
	{
		TextParams params;

		params.Color = component.Color;
		params.Kerning = component.Kerning;
		params.LineSpacing = component.LineSpacing;


		DrawString(string, component.Font, transform, params, entityID);

	}


}