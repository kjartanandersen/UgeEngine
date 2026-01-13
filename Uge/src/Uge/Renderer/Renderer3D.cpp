#include "ugpch.h"
#include "Renderer3D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

namespace Uge
{


	struct Renderer3DStorage
	{
		Ref<VertexArray> m_squareVA;
		Ref<Shader> m_textureShader;
		Ref<Texture2D> m_whiteTexture;


	};

	static Renderer3DStorage* m_data;


	void Uge::Renderer3D::Init()
	{
		UG_PROFILE_FUNCTION();

		m_data = new Renderer3DStorage();
		m_data->m_squareVA = VertexArray::Create();

		float CubeVertices[] =
{
	/* Vertices */				/* Texture Coordinates */
    // ---------- Front (+Z)
    -0.5f, -0.5f,  0.5f,		0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,		1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,		1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,		0.0f, 1.0f,

    // ---------- Back (−Z)
     0.5f, -0.5f, -0.5f,		0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,		1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,		1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,		0.0f, 1.0f,

    // ---------- Left (−X)
    -0.5f, -0.5f, -0.5f,		0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,		1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,		1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,		0.0f, 1.0f,

    // ---------- Right (+X)
     0.5f, -0.5f,  0.5f,		0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,		1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,		1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,		0.0f, 1.0f,

    // ---------- Top (+Y)
    -0.5f,  0.5f,  0.5f,		0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,		1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,		1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,		0.0f, 1.0f,

    // ---------- Bottom(−Y)
    -0.5f, -0.5f, -0.5f,		0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,		1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,		1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,		0.0f, 1.0f,
};


		Ref<VertexBuffer> squareVB;
		squareVB = VertexBuffer::Create(CubeVertices, sizeof(CubeVertices));

		BufferLayout squareVBlayout =
		{
			{ ShaderDataType::Float3, "a_Position"},
			{ ShaderDataType::Float2, "a_TextCoord"}
		};

		squareVB->SetLayout(squareVBlayout);
		m_data->m_squareVA->AddVertexBuffer(squareVB);

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

		m_data->m_squareVA->SetIndexBuffer(squareIB);

		m_data->m_whiteTexture = Texture2D::Create(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		m_data->m_whiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));


		m_data->m_textureShader = Shader::Create("assets/shaders/Texture.glsl");
		m_data->m_textureShader->Bind();
		m_data->m_textureShader->SetInt("u_Texture", 0);


	}

	void Renderer3D::Shutdown()
	{
		UG_PROFILE_FUNCTION();

		delete m_data;
	}

	void Renderer3D::BeginScene(const PerspectiveCamera& camera)
	{
		UG_PROFILE_FUNCTION();

		m_data->m_textureShader->Bind();
		m_data->m_textureShader->SetMat4(
			"u_ViewProjection", camera.GetViewProjectionMatrix());
	}

	void Renderer3D::EndScene()
	{
		UG_PROFILE_FUNCTION();
	}

	void Renderer3D::DrawCube(const glm::vec3& position, float rotation, const glm::vec3& size, const glm::vec4& color)
	{
		UG_PROFILE_FUNCTION();

		m_data->m_textureShader->SetFloat4(
			"u_Color", color);

		m_data->m_whiteTexture->Bind();

		glm::mat4 transform =
			glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f }) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, size.z });

		m_data->m_textureShader->SetMat4("u_ModelMatrix", transform);


		m_data->m_squareVA->Bind();
		RenderCommand::DrawIndexed(m_data->m_squareVA);

	}

	void Renderer3D::DrawCube(const glm::vec3& position, float rotation, const glm::vec3& size, Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{

		UG_PROFILE_FUNCTION();

		m_data->m_textureShader->SetFloat4("u_Color", tintColor);
		m_data->m_textureShader->SetFloat("u_TilingFactor", tilingFactor);
		texture->Bind();



		glm::mat4 transform =
			glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0, 0, 1)) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, size.z });

		m_data->m_textureShader->SetMat4("u_ModelMatrix", transform);



		m_data->m_squareVA->Bind();
		RenderCommand::DrawIndexed(m_data->m_squareVA);
		texture->UnBind();


	}



}