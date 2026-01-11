#pragma once

#include "PerspectiveCamera.h"
#include "Texture.h"

namespace Uge
{

	class Renderer3D
	{

	public:
		static void Init();
		static void Shutdown();


		static void BeginScene(const PerspectiveCamera& camera);
		static void EndScene();

		// Primitives
		static void DrawCube(const glm::vec3& position, float rotation, const glm::vec3& size, const glm::vec4& color);




	};


}
