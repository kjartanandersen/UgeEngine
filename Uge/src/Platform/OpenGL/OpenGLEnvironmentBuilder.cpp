#include <ugpch.h>
#include "OpenGLEnvironmentBuilder.h"

#include "Uge/Renderer/Buffer.h"
#include "Uge/Renderer/RenderCommand.h"
#include "Uge/Renderer/Shader.h"
#include "Uge/Renderer/UniformBuffer.h"
#include "Uge/Renderer/VertexArray.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Uge
{

	namespace
	{
		// Mirrors CubeRenderData in the environment build shaders. std140 rounds the block up
		// to a multiple of 16, hence the padding after the single float.
		struct CubeRenderData
		{
			glm::mat4 ViewProjection;
			float Roughness;
			float Padding[3];
		};

		constexpr uint32_t k_environmentSize = 1024;
		constexpr uint32_t k_irradianceSize = 32;
		constexpr uint32_t k_prefilterSize = 128;
		constexpr uint32_t k_prefilterMipCount = 5;
		constexpr uint32_t k_brdfLutSize = 512;

		// 90 degrees of field of view is exactly one cube face. The near and far planes only
		// have to bracket the unit cube.
		const glm::mat4 k_captureProjection =
			glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

		// Order matches GL_TEXTURE_CUBE_MAP_POSITIVE_X + i. The up vectors are negated on the
		// horizontal faces because a cubemap's texture space is left-handed and flipped
		// vertically relative to the world - the standard, deeply unintuitive convention.
		const glm::mat4 k_captureViews[6] =
		{
			glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
			glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
		};

		Ref<VertexArray> CreateUnitCube()
		{
			// Wound so the faces point inwards: every pass renders the cube from its centre.
			constexpr float vertices[] =
			{
				-1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f
			};

			constexpr uint32_t indices[] =
			{
				0, 2, 1,  0, 3, 2,   // back
				4, 5, 6,  4, 6, 7,   // front
				0, 7, 3,  0, 4, 7,   // left
				1, 2, 6,  1, 6, 5,   // right
				3, 6, 2,  3, 7, 6,   // top
				0, 1, 5,  0, 5, 4    // bottom
			};

			Ref<VertexArray> vertexArray = VertexArray::Create();

			Ref<VertexBuffer> vertexBuffer =
				VertexBuffer::Create(const_cast<float*>(vertices), sizeof(vertices));
			vertexBuffer->SetLayout({ { ShaderDataType::Float3, "a_Position" } });
			vertexArray->AddVertexBuffer(vertexBuffer);

			vertexArray->SetIndexBuffer(
				IndexBuffer::Create(const_cast<uint32_t*>(indices), sizeof(indices) / sizeof(uint32_t)));

			return vertexArray;
		}

		Ref<VertexArray> CreateFullscreenTriangle()
		{
			constexpr float vertices[] =
			{
				-1.0f, -1.0f,  0.0f, 0.0f,
				 3.0f, -1.0f,  2.0f, 0.0f,
				-1.0f,  3.0f,  0.0f, 2.0f
			};
			constexpr uint32_t indices[] = { 0, 1, 2 };

			Ref<VertexArray> vertexArray = VertexArray::Create();

			Ref<VertexBuffer> vertexBuffer =
				VertexBuffer::Create(const_cast<float*>(vertices), sizeof(vertices));
			vertexBuffer->SetLayout({
				{ ShaderDataType::Float2, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			});
			vertexArray->AddVertexBuffer(vertexBuffer);

			vertexArray->SetIndexBuffer(
				IndexBuffer::Create(const_cast<uint32_t*>(indices), sizeof(indices) / sizeof(uint32_t)));

			return vertexArray;
		}

		// Renders the six faces of one cubemap mip level with the given shader bound.
		void RenderCubeFaces(uint32_t framebuffer, const Ref<TextureCube>& target, uint32_t mipLevel,
			uint32_t mipSize, float roughness, const Ref<VertexArray>& cube,
			const Ref<UniformBuffer>& uniformBuffer)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			glViewport(0, 0, mipSize, mipSize);

			for (uint32_t face = 0; face < 6; ++face)
			{
				CubeRenderData data{};
				data.ViewProjection = k_captureProjection * k_captureViews[face];
				data.Roughness = roughness;
				uniformBuffer->SetData(&data, sizeof(data));

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, target->GetRendererID(), mipLevel);

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				RenderCommand::DrawIndexed(cube);
			}
		}
	}

	Ref<Environment> OpenGLEnvironmentBuilder::Build(const Ref<Texture2D>& equirectangular)
	{
		UG_PROFILE_FUNCTION();

		if (!equirectangular)
		{
			return nullptr;
		}

		Ref<Environment> environment = CreateRef<Environment>();

		Ref<VertexArray> cube = CreateUnitCube();
		Ref<UniformBuffer> uniformBuffer = UniformBuffer::Create(sizeof(CubeRenderData), 6);

		Ref<Shader> equirectShader = Shader::Create("assets/shaders/EquirectToCubemap.glsl");
		Ref<Shader> irradianceShader = Shader::Create("assets/shaders/IrradianceConvolution.glsl");
		Ref<Shader> prefilterShader = Shader::Create("assets/shaders/PrefilterEnvironment.glsl");
		Ref<Shader> brdfShader = Shader::Create("assets/shaders/BrdfLut.glsl");

		uint32_t framebuffer = 0;
		glCreateFramebuffers(1, &framebuffer);

		// The cube is drawn from the inside. Culling is never enabled anywhere in the engine, so
		// this is a no-op today and only guards against that changing.
		glDisable(GL_CULL_FACE);
		RenderCommand::SetDepthTest(false);

		// ---- 1. Project the equirectangular image onto a cubemap ----

		TextureCubeSpecification environmentSpec;
		environmentSpec.Size = k_environmentSize;
		environmentSpec.Format = ImageFormat::RGBA16F;

		// Mipped so the prefilter pass can sample a coarser level for its wider lobes, which is
		// what stops a small bright sun turning into a field of fireflies.
		environmentSpec.GenerateMips = true;
		environment->Skybox = TextureCube::Create(environmentSpec);

		equirectShader->Bind();
		equirectangular->Bind(0);
		RenderCubeFaces(framebuffer, environment->Skybox, 0, k_environmentSize, 0.0f, cube, uniformBuffer);
		environment->Skybox->GenerateMips();

		// ---- 2. Cosine-convolve into the diffuse irradiance map ----

		TextureCubeSpecification irradianceSpec;
		irradianceSpec.Size = k_irradianceSize;
		irradianceSpec.Format = ImageFormat::RGBA16F;
		irradianceSpec.GenerateMips = false;
		environment->Irradiance = TextureCube::Create(irradianceSpec);

		irradianceShader->Bind();
		environment->Skybox->Bind(0);
		RenderCubeFaces(framebuffer, environment->Irradiance, 0, k_irradianceSize, 0.0f, cube, uniformBuffer);

		// ---- 3. Prefilter the specular map, one roughness per mip ----

		TextureCubeSpecification prefilterSpec;
		prefilterSpec.Size = k_prefilterSize;
		prefilterSpec.Format = ImageFormat::RGBA16F;
		prefilterSpec.GenerateMips = true;

		// Exactly as many levels as the loop below writes. A full chain would leave levels 5-7
		// of a 128px cube undefined, and Model.glsl maps roughness 1.0 onto the last level -
		// so every rough surface in the scene would sample uninitialized memory.
		prefilterSpec.MipLevels = k_prefilterMipCount;
		environment->Prefiltered = TextureCube::Create(prefilterSpec);

		prefilterShader->Bind();
		environment->Skybox->Bind(0);

		for (uint32_t mip = 0; mip < k_prefilterMipCount; ++mip)
		{
			const uint32_t mipSize = k_prefilterSize >> mip;
			const float roughness = (float)mip / (float)(k_prefilterMipCount - 1);

			RenderCubeFaces(framebuffer, environment->Prefiltered, mip, mipSize, roughness, cube, uniformBuffer);
		}

		// ---- 4. Integrate the BRDF lookup table ----

		TextureSpecification brdfSpec;
		brdfSpec.Width = k_brdfLutSize;
		brdfSpec.Height = k_brdfLutSize;
		brdfSpec.Format = ImageFormat::RG16F;
		brdfSpec.GenerateMips = false;
		environment->BrdfLut = Texture2D::Create(brdfSpec);

		// Clamped rather than repeated: this is a lookup table, and wrapping at the edges would
		// return the reflectance for the opposite view angle.
		glTextureParameteri(environment->BrdfLut->GetRendererID(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(environment->BrdfLut->GetRendererID(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		Ref<VertexArray> fullscreenTriangle = CreateFullscreenTriangle();

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			environment->BrdfLut->GetRendererID(), 0);
		glViewport(0, 0, k_brdfLutSize, k_brdfLutSize);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		brdfShader->Bind();
		RenderCommand::DrawIndexed(fullscreenTriangle);

		// ---- Restore ----

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &framebuffer);

		RenderCommand::SetDepthTest(true);

		return environment;
	}

}
