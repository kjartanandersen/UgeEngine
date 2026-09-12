#include <ugpch.h>
#include "FullscreenTriangle.h"

#include "Uge/Renderer/Buffer.h"

namespace Uge
{

	const Ref<VertexArray>& GetFullscreenTriangle()
	{
		// Created on first use rather than at startup: the graphics context does not exist
		// when static initializers run.
		static Ref<VertexArray> s_triangle;

		if (!s_triangle)
		{
			constexpr float vertices[] =
			{
				// position      texcoord
				-1.0f, -1.0f,    0.0f, 0.0f,
				 3.0f, -1.0f,    2.0f, 0.0f,
				-1.0f,  3.0f,    0.0f, 2.0f
			};

			constexpr uint32_t indices[] = { 0, 1, 2 };

			s_triangle = VertexArray::Create();

			Ref<VertexBuffer> vertexBuffer =
				VertexBuffer::Create(const_cast<float*>(vertices), sizeof(vertices));
			vertexBuffer->SetLayout({
				{ ShaderDataType::Float2, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			});
			s_triangle->AddVertexBuffer(vertexBuffer);

			s_triangle->SetIndexBuffer(
				IndexBuffer::Create(const_cast<uint32_t*>(indices), sizeof(indices) / sizeof(uint32_t)));
		}

		return s_triangle;
	}

}
