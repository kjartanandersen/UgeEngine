#include "ugpch.h"
#include "Renderer3D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"


#include <glm/gtc/matrix_transform.hpp>

namespace Uge
{


	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
	};

	struct Renderer3DData
	{
		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;


		Ref<VertexArray> QuadVA;
		Ref<VertexBuffer> QuadVB;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;
		Ref<Texture2D> CheckerboardTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;


	};

	static Renderer3DData m_data;


	void Uge::Renderer3D::Init()
	{
		UG_PROFILE_FUNCTION();

		
		m_data.QuadVA = VertexArray::Create();

		 float CubeVertices[] =
		{
			// ---------- Front (+Z)
			-1.f, -1.f,  1.f,   0.f, 0.f,
			 1.f, -1.f,  1.f,   1.f, 0.f,
			 1.f,  1.f,  1.f,   1.f, 1.f,
			-1.f,  1.f,  1.f,   0.f, 1.f,

			// ---------- Back (−Z)
			 1.f, -1.f, -1.f,   0.f, 0.f,
			-1.f, -1.f, -1.f,   1.f, 0.f,
			-1.f,  1.f, -1.f,   1.f, 1.f,
			 1.f,  1.f, -1.f,   0.f, 1.f,

			 // ---------- Left (−X)
			 -1.f, -1.f, -1.f,   0.f, 0.f,
			 -1.f, -1.f,  1.f,   1.f, 0.f,
			 -1.f,  1.f,  1.f,   1.f, 1.f,
			 -1.f,  1.f, -1.f,   0.f, 1.f,

			 // ---------- Right (+X)
			  1.f, -1.f,  1.f,   0.f, 0.f,
			  1.f, -1.f, -1.f,   1.f, 0.f,
			  1.f,  1.f, -1.f,   1.f, 1.f,
			  1.f,  1.f,  1.f,   0.f, 1.f,

			  // ---------- Top (+Y)
			  -1.f,  1.f,  1.f,   0.f, 0.f,
			   1.f,  1.f,  1.f,   1.f, 0.f,
			   1.f,  1.f, -1.f,   1.f, 1.f,
			  -1.f,  1.f, -1.f,   0.f, 1.f,

			  // ---------- Bottom (−Y)
			  -1.f, -1.f, -1.f,   0.f, 0.f,
			   1.f, -1.f, -1.f,   1.f, 0.f,
			   1.f, -1.f,  1.f,   1.f, 1.f,
			  -1.f, -1.f,  1.f,   0.f, 1.f,
		};


		Ref<VertexBuffer> squareVB;
		squareVB = VertexBuffer::Create(CubeVertices, sizeof(CubeVertices));

		BufferLayout squareVBlayout =
		{
			{ ShaderDataType::Float3, "a_Position"},
			{ ShaderDataType::Float2, "a_TextCoord"}
		};

		squareVB->SetLayout(squareVBlayout);
		m_data.QuadVA->AddVertexBuffer(squareVB);

		uint32_t CubeIndices[] =
		{
			 0,  1,  2,   2,  3,  0,   // Front
			 4,  5,  6,   6,  7,  4,   // Back
			 8,  9, 10,  10, 11,  8,   // Left
			12, 13, 14,  14, 15, 12,   // Right
			16, 17, 18,  18, 19, 16,   // Top
			20, 21, 22,  22, 23, 20    // Bottom
		};

		Ref<IndexBuffer> squareIB;

		squareIB = IndexBuffer::Create(CubeIndices, sizeof(CubeIndices) / sizeof(uint32_t));

		m_data.QuadVA->SetIndexBuffer(squareIB);

		m_data.WhiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		m_data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		
		m_data.CheckerboardTexture = Texture2D::Create("C:\\Programming\\c++\\GameEngines\\Uge\\Sandbox\\assets\\textures\\Checkerboard.png");


		m_data.TextureShader = Shader::Create("assets/shaders/FlatTexture3D.glsl");
		m_data.TextureShader->Bind();
		//m_data.TextureShader->SetInt("u_Texture", 0);


	}

	void Renderer3D::Shutdown()
	{
		UG_PROFILE_FUNCTION();

	}

	void Renderer3D::BeginScene(const PerspectiveCamera& camera)
	{
		UG_PROFILE_FUNCTION();

		m_data.TextureShader->Bind();
		//m_data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
	}

	void Renderer3D::EndScene()
	{
		UG_PROFILE_FUNCTION();
	}

	void Renderer3D::DrawCube(const glm::vec3& position, float rotation, const glm::vec3& size, const glm::vec4& color)
	{
		UG_PROFILE_FUNCTION();

		m_data.TextureShader->SetFloat4(
			"u_Color", color);

		m_data.CheckerboardTexture->Bind();

		glm::mat4 transform =
			glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f }) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, size.z });

		m_data.TextureShader->SetMat4("u_ModelMatrix", transform);


		m_data.QuadVA->Bind();
		RenderCommand::DrawIndexed(m_data.QuadVA);

	}

	void Renderer3D::DrawCube(const glm::vec3& position, float rotation, const glm::vec3& size, Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{

		UG_PROFILE_FUNCTION();

		m_data.CheckerboardTexture->Bind();

		m_data.TextureShader->SetFloat4("u_Color", tintColor);
		m_data.TextureShader->SetFloat("u_TilingFactor", tilingFactor);
		texture->Bind();



		glm::mat4 transform =
			glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 0, 1)) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, size.z });

		m_data.TextureShader->SetMat4("u_ModelMatrix", transform);



		m_data.QuadVA->Bind();
		RenderCommand::DrawIndexed(m_data.QuadVA);
		texture->UnBind();


	}



}