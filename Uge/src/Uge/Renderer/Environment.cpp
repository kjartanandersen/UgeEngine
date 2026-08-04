#include <ugpch.h>
#include "Environment.h"

#include "Renderer.h"

#include "Uge/Asset/TextureImporter.h"
#include "Platform/OpenGL/OpenGLEnvironmentBuilder.h"

namespace Uge
{

	Ref<Environment> Environment::Create(const std::filesystem::path& path)
	{
		UG_PROFILE_FUNCTION();

		Ref<Texture2D> equirectangular = TextureImporter::LoadTextureHDR(path);
		if (!equirectangular)
		{
			return nullptr;
		}

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:    UG_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::OpenGL:  return OpenGLEnvironmentBuilder::Build(equirectangular);
		}

		UG_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
