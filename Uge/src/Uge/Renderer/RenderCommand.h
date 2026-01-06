#pragma once

#include "RendererAPI.h"


namespace Uge
{


	class RenderCommand
	{

	public :

		inline static void Init()
		{
			m_rendererAPI->Init();
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{

			m_rendererAPI->SetViewport(x, y, width, height);
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			m_rendererAPI->SetClearColor(color);
		}
		inline static void Clear() 
		{
			m_rendererAPI->Clear();
		}


		inline static void DrawIndexed(const Ref<VertexArray> vertexArray)
		{
			m_rendererAPI->DrawIndexed(vertexArray);
		}


	private:
		static RendererAPI* m_rendererAPI;



	};


}


