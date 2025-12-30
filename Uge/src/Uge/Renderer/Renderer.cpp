#include <ugpch.h>
#include "Renderer.h"


namespace Uge
{

	Renderer::SceneData* Renderer::m_sceneData = new Renderer::SceneData;

	void Renderer::BeginScene(OrthographicCamera& camera)
	{

		m_sceneData->viewProjectionMatrix = camera.GetViewProjectionMatrix();

	}

	void Renderer::EndScene()
	{




	}

	void Renderer::Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray> vertexArray)
	{
		shader->Bind();
		shader->UploadUniformMat4("u_ViewProjection", m_sceneData->viewProjectionMatrix);

		vertexArray->Bind();
		RenderCommand::DrawIndexed(vertexArray);


	}

}

