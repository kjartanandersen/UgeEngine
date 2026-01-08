#include "ugpch.h"
#include "Renderer2D.h"

#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"

#include "glm/glm.hpp"
#include <glm/ext/matrix_transform.hpp>

namespace Uge
{

	struct Renderer2DStorage
	{
		Ref<VertexArray> m_squareVA;
		Ref<Shader> m_flatColorShader;
		Ref<Shader> m_textureShader;


	};

	static Renderer2DStorage* m_data;


	void Renderer2D::Init()
	{

		m_data = new Renderer2DStorage();
		m_data->m_squareVA = VertexArray::Create();

		float squareVertices[5 * 4] =
		{
			/* Vertices */		 /* Texture Coordinates */
		   -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,
			0.5f, -0.5f, 0.0f,	 1.0f, 0.0f,
			0.5f,  0.5f, 0.0f,	 1.0f, 1.0f,
		   -0.5f,  0.5f, 0.0f,	 0.0f, 1.0f
		};


		Ref<VertexBuffer> squareVB;
		squareVB = VertexBuffer::Create(squareVertices, sizeof(squareVertices));

		BufferLayout squareVBlayout =
		{
			{ ShaderDataType::Float3, "a_Position"},
			{ ShaderDataType::Float2, "a_TextCoord"}
		};

		squareVB->SetLayout(squareVBlayout);
		m_data->m_squareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

		Ref<IndexBuffer> squareIB;

		squareIB = IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));

		m_data->m_squareVA->SetIndexBuffer(squareIB);


		m_data->m_flatColorShader = Shader::Create("assets/shaders/FlatColorShader.glsl");
		m_data->m_textureShader = Shader::Create("assets/shaders/Texture.glsl");
		m_data->m_textureShader->Bind();
		m_data->m_textureShader->SetInt("u_Texture", 0);



	}

	void Renderer2D::Shutdown()
	{

		delete m_data;

	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{

		m_data->m_flatColorShader->Bind();
		m_data->m_flatColorShader->SetMat4(
			"u_ViewProjection", camera.GetViewProjectionMatrix());

		m_data->m_textureShader->Bind();
		m_data->m_textureShader->SetMat4(
			"u_ViewProjection", camera.GetViewProjectionMatrix());
		


	}

	void Renderer2D::EndScene()
	{
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const glm::vec4& color)
	{

		DrawQuad({ position.x, position.y, 0.0f }, rotation, size, color );


	}

	void Renderer2D::DrawQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const glm::vec4& color)
	{

		m_data->m_flatColorShader->Bind();

		m_data->m_flatColorShader->SetFloat4(
			"u_Color", color);

		glm::mat4 transform = 
			glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f}) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		m_data->m_flatColorShader->SetMat4("u_ModelMatrix", transform);

		

		m_data->m_squareVA->Bind();
		RenderCommand::DrawIndexed(m_data->m_squareVA);


	}

	void Renderer2D::DrawQuad(const glm::vec2& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture)
	{

		DrawQuad({ position.x, position.y, 0.0f }, rotation, size, texture);

	}

	void Renderer2D::DrawQuad(const glm::vec3& position, float rotation, const glm::vec2& size, const Ref<Texture2D>& texture)
	{

		m_data->m_textureShader->Bind();

		

		glm::mat4 transform = 
			glm::translate(glm::mat4(1.0f), position) *
			glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f}) *
			glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		m_data->m_textureShader->SetMat4("u_ModelMatrix", transform);

		texture->Bind();
		

		m_data->m_squareVA->Bind();
		RenderCommand::DrawIndexed(m_data->m_squareVA);


	}

}