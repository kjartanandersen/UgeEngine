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

	/** @brief A vertex of a text quad. */
	struct TextVertex
	{
		glm::vec3 Position; ///< World-space position.
		glm::vec4 Color; ///< RGBA colour.
		glm::vec2 TexCoord; ///< Coordinates in the font atlas.

		// TODO: bg color for outline/bg

		// Editor Only
		int EntityID; ///< The entity, for mouse picking.
	};

	/** @brief A vertex of a line. */
	struct LineVertex
	{
		glm::vec3 Position; ///< World-space position.
		glm::vec4 Color; ///< RGBA colour.

		// Editor only
		int EntityID; ///< The entity, for mouse picking.
	};

	/**
	 * @brief Uge::Renderer2D's state. Vertices are written to CPU arrays and uploaded once
	 * per batch.
	 */
	struct Renderer2DData
	{
		static const uint32_t MaxQuads = 20000;				///< Quads per batch.
		static const uint32_t MaxLineVertices = 20000;		///< Line vertices per batch.
		static const uint32_t MaxVertices = MaxQuads * 4;	///< Quad vertices per batch.
		static const uint32_t MaxIndices = MaxQuads * 6;	///< Quad indices per batch.
		static const uint32_t MaxTextureSlots = 32;			///< Texture slots per batch.

		Ref<VertexArray> QuadVA; ///< Quad vertex array.
		Ref<VertexBuffer> QuadVB; ///< Quad vertex buffer.
		Ref<Shader> TextureShader; ///< Quad shader.
		Ref<Texture2D> WhiteTexture; ///< 1x1 white texture in slot 0, for untextured quads.

		Ref<VertexArray> TextVA; ///< Text vertex array.
		Ref<VertexBuffer> TextVB; ///< Text vertex buffer.
		Ref<Shader> TextShader; ///< Text shader.

		Ref<VertexArray> LineVA;	///< Line vertex array.
		Ref<VertexBuffer> LineVB;	///< Line vertex buffer.
		Ref<Shader> LineShader;		///< Line shader.

		uint32_t QuadIndexCount = 0; ///< Quad indices in the batch.
		Vertex* VertexBufferBase = nullptr; ///< Start of the quad vertex array.
		Vertex* VertexBufferPtr = nullptr; ///< Next quad vertex to write.

		uint32_t TextIndexCount = 0; ///< Text indices in the batch.
		TextVertex* TextVertexBufferBase = nullptr; ///< Start of the text vertex array.
		TextVertex* TextVertexBufferPtr = nullptr; ///< Next text vertex to write.

		uint32_t LineVertexCount = 0;					///< Line vertices in the batch.
		LineVertex* LineVertexBufferBase = nullptr;		///< Start of the line vertex array.
		LineVertex* LineVertexBufferPtr = nullptr;		///< Next line vertex to write.
		float LineWidth = 1;							///< Line width in pixels.

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots; ///< The batch's textures.
		uint32_t TextureSlotIndex = 1; ///< Next free slot.

		Ref<Texture2D> FontAtlasTexture; ///< The text batch's font atlas.

		glm::vec4 VertexPositions[4] = { {} }; ///< Unit quad corners.
		

		/** @brief The camera uniform block. */
		struct CameraData
		{
			glm::mat4 ViewProjection; ///< View-projection matrix.
		};

		CameraData CameraBuffer = { {} }; ///< The camera block.
		Ref<UniformBuffer> CameraUniformBuffer; ///< Its uniform buffer, at binding 0.


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


		m_data.TextVB = VertexBuffer::Create(m_data.MaxVertices * sizeof(TextVertex));
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

		// Lines
		m_data.LineVA = VertexArray::Create();
		m_data.LineVB = VertexBuffer::Create(m_data.MaxLineVertices * sizeof(LineVertex));
		BufferLayout lineVBlayout =
		{
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Int,    "a_EntityID"     }
		};
		m_data.LineVB->SetLayout(lineVBlayout);
		m_data.LineVA->AddVertexBuffer(m_data.LineVB);

		m_data.LineVertexBufferBase = new LineVertex[m_data.MaxLineVertices];


		// Indices
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

		// Textures
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

		// Shaders
		m_data.TextureShader	= Shader::Create("assets/shaders/Texture.glsl");
		m_data.TextShader		= Shader::Create("assets/shaders/Text.glsl");
		m_data.LineShader		= Shader::Create("assets/shaders/Line.glsl");


		
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

		// The vertex arrays are new[]; the rest are Refs, released by the reset.
		delete[] m_data.VertexBufferBase;
		delete[] m_data.TextVertexBufferBase;
		delete[] m_data.LineVertexBufferBase;

		m_data = Renderer2DData();

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

		if (m_data.LineVertexCount)
		{

			uint32_t dataSize = (uint32_t)((uint8_t*)m_data.LineVertexBufferPtr - (uint8_t*)m_data.LineVertexBufferBase);
			m_data.LineVB->SetData(m_data.LineVertexBufferBase, dataSize);

			m_data.LineShader->Bind();
			
			RenderCommand::SetLineWidth(m_data.LineWidth);
			RenderCommand::DrawLines(m_data.LineVA, m_data.LineVertexCount);

		}

	}

	float Renderer2D::GetLineWidth()
	{
		return m_data.LineWidth;
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{

		DrawQuad({ position.x, position.y, 0.0f }, size, color );


	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		
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

		constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		const glm::vec2* textureCoords = subTexture->GetTextCoords();
		const Ref<Texture2D> texture = subTexture->GetTexture();

		if (m_data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{

			NextBatch();

		}


		float textureIndex = GetTextureSlot(texture);

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

		if (m_data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{

			NextBatch();

		}

		constexpr size_t VertexCount = 4;
		const float texIndex = 0.0f; // White texture
		const float tilingFactor = 1.0f;
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
		UG_CORE_VERIFY(texture);

		if (m_data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{

			NextBatch();

		}


		constexpr size_t VertexCount = 4;
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		float texIndex = GetTextureSlot(texture);

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
		if (m_data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{

			NextBatch();

		}

		const float texIndex = 0.0f; // White texture
		const float tilingFactor = 1.0f;

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
		if (m_data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{

			NextBatch();

		}

		constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

		float textureIndex = GetTextureSlot(texture);

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

		constexpr glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		const glm::vec2* textureCoords = subTexture->GetTextCoords();
		const Ref<Texture2D> texture = subTexture->GetTexture();

		if (m_data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{

			NextBatch();

		}


		float textureIndex = GetTextureSlot(texture);

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

		m_data.LineVertexCount = 0;
		m_data.LineVertexBufferPtr = m_data.LineVertexBufferBase;

		m_data.TextureSlotIndex = 1;

	}

	void Renderer2D::NextBatch()
	{

		Flush();
		StartBatch();

	}

	float Renderer2D::GetTextureSlot(const Ref<Texture2D>& texture)
	{
		// From 0, so the white texture is found in slot 0.
		for (uint32_t i = 0; i < m_data.TextureSlotIndex; i++)
		{
			if (*m_data.TextureSlots[i] == *texture)
			{
				return (float)i;
			}
		}

		// The shader has MaxTextureSlots samplers; when they are all taken, start a new batch.
		if (m_data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
		{
			NextBatch();
		}

		const uint32_t slot = m_data.TextureSlotIndex++;
		m_data.TextureSlots[slot] = texture;
		return (float)slot;
	}

	void Renderer2D::DrawSprite(const glm::mat4& transform, const SpriteRendererComponent& src, int entityID)
	{
		Ref<Texture2D> texture;
		if (src.Texture)
		{
			texture = AssetManager::GetAsset<Texture2D>(src.Texture);
		}

		// A sprite whose texture doesn't resolve is drawn in its plain colour.
		if (texture)
		{
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

			// The glyph's four corners.
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

	void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, int entityID)
	{

		if (m_data.LineVertexCount >= Renderer2DData::MaxLineVertices)
		{
			NextBatch();
		}

		m_data.LineVertexBufferPtr->Position = p0;
		m_data.LineVertexBufferPtr->Color = color;
		m_data.LineVertexBufferPtr->EntityID = entityID;
		m_data.LineVertexBufferPtr++;

		m_data.LineVertexBufferPtr->Position = p1;
		m_data.LineVertexBufferPtr->Color = color;
		m_data.LineVertexBufferPtr->EntityID = entityID;
		m_data.LineVertexBufferPtr++;

		m_data.LineVertexCount += 2;

	}

	void Renderer2D::SetLineWidth(float width)
	{
		m_data.LineWidth = width;
	}



}