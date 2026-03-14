#include <ugpch.h>
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"
#include "Renderer2D.h"
#include "Renderer3D.h"

namespace Uge
{

	Renderer::SceneData* Renderer::m_sceneData = new Renderer::SceneData;

	void Renderer::Init(bool is3D)
	{
		UG_PROFILE_FUNCTION();
		RenderCommand::Init();

		if (is3D)
			Renderer3D::Init();
		else
			Renderer2D::Init();


	}

	void Renderer::Shutdown()
	{

		Renderer2D::Shutdown();

	}

	void Renderer::OnWindowResize(uint32_t width, uint32_t height)
	{

		RenderCommand::SetViewport(0, 0, width, height);


	}


	void Renderer::BeginScene(Camera& camera)
	{

		//m_sceneData->viewProjectionMatrix = camera.GetViewProjectionMatrix();

	}


	void Renderer::EndScene()
	{




	}

	void Renderer::Submit(const Ref<Shader>& shader,
		const Ref<VertexArray> vertexArray, const glm::mat4& transform)
	{
		shader->Bind();
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", m_sceneData->viewProjectionMatrix);
		std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ModelMatrix", transform);
		


		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);


	}

}

