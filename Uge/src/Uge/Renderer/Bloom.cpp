#include <ugpch.h>
#include "Bloom.h"

#include "Uge/Renderer/FullscreenTriangle.h"
#include "Uge/Renderer/PostProcess.h"
#include "Uge/Renderer/RenderCommand.h"

#include <glm/glm.hpp>

namespace Uge
{

	namespace
	{
		// Mirrors BloomData in the bloom shaders. std140 rounds the block up to a multiple of
		// 16 bytes, which the two vec2-worth of members below already satisfy exactly.
		struct BloomUniformData
		{
			glm::vec2 TexelSize;
			float Threshold;
			float Knee;
			float FilterRadius;
			int32_t IsFirstPass;
			float Padding[2];
		};

		// Six halvings of a 1080p viewport reaches roughly 16 pixels across, which is a wide
		// enough radius for a convincing glow without the coarsest level becoming so small that
		// its texels are visible as blocks when upsampled.
		constexpr uint32_t k_maxLevels = 6;

		// Below this a level carries too little detail to be worth a pass.
		constexpr uint32_t k_minLevelSize = 8;

		// In texels of the level being sampled. Widening in the source's units rather than the
		// target's is what makes the spread grow with each coarser level.
		constexpr float k_filterRadius = 1.0f;
	}

	Bloom::BloomData Bloom::s_bloomData;

	void Bloom::EnsureResources()
	{
		if (s_bloomData.Initialized)
		{
			return;
		}

		s_bloomData.DownsampleShader = Shader::Create("assets/shaders/BloomDownsample.glsl");
		s_bloomData.UpsampleShader = Shader::Create("assets/shaders/BloomUpsample.glsl");

		// Binding 8: 0-7 are taken by the 2D camera, model, material, mesh camera, light,
		// tonemap, environment build and skybox blocks respectively.
		s_bloomData.SettingsUniformBuffer = UniformBuffer::Create(sizeof(BloomUniformData), 8);

		s_bloomData.Initialized = true;
	}

	void Bloom::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
		{
			return;
		}

		if (s_bloomData.Width == width && s_bloomData.Height == height)
		{
			return;
		}

		s_bloomData.Width = width;
		s_bloomData.Height = height;
		s_bloomData.Levels.clear();
		s_bloomData.HasResult = false;

		uint32_t levelWidth = width;
		uint32_t levelHeight = height;

		for (uint32_t level = 0; level < k_maxLevels; ++level)
		{
			levelWidth /= 2;
			levelHeight /= 2;

			if (levelWidth < k_minLevelSize || levelHeight < k_minLevelSize)
			{
				break;
			}

			// RGBA16F, like the scene target: the whole point is to carry values above 1.0
			// through to the resolve, and an 8-bit level would clamp them away here.
			FramebufferSpecification spec{ levelWidth, levelHeight };
			spec.Attachments = { FramebufferTextureFormat::RGBA16F };

			s_bloomData.Levels.push_back(Framebuffer::Create(spec));
		}
	}

	void Bloom::Render(const Ref<Framebuffer>& scene, const RenderSettings& settings)
	{
		UG_PROFILE_FUNCTION();

		s_bloomData.HasResult = false;

		if (!scene || !settings.BloomEnabled)
		{
			return;
		}

		Resize(scene->GetSpecification().Width, scene->GetSpecification().Height);

		if (s_bloomData.Levels.empty())
		{
			return;
		}

		EnsureResources();

		const Ref<VertexArray>& triangle = GetFullscreenTriangle();

		RenderCommand::SetDepthTest(false);

		// Each downsample replaces its target outright; leaving alpha blending on would
		// composite against whatever the previous frame left there.
		RenderCommand::SetBlendMode(BlendMode::None);

		BloomUniformData uniformData{};
		uniformData.Threshold = settings.BloomThreshold;
		uniformData.Knee = glm::max(settings.BloomKnee, 0.0001f);
		uniformData.FilterRadius = k_filterRadius;

		// ---- Downsample, thresholding on the way into the first level ----

		s_bloomData.DownsampleShader->Bind();

		for (size_t level = 0; level < s_bloomData.Levels.size(); ++level)
		{
			const bool isFirst = (level == 0);

			// Texel size of the source, not the target: the 13-tap kernel is expressed in the
			// units of the texture it reads from.
			const FramebufferSpecification& sourceSpec = isFirst
				? scene->GetSpecification()
				: s_bloomData.Levels[level - 1]->GetSpecification();

			uniformData.TexelSize = glm::vec2(1.0f / (float)sourceSpec.Width, 1.0f / (float)sourceSpec.Height);
			uniformData.IsFirstPass = isFirst ? 1 : 0;
			s_bloomData.SettingsUniformBuffer->SetData(&uniformData, sizeof(uniformData));

			if (isFirst)
			{
				scene->BindColorAttachment(0, 0);
			}
			else
			{
				s_bloomData.Levels[level - 1]->BindColorAttachment(0, 0);
			}

			s_bloomData.Levels[level]->Bind();
			RenderCommand::DrawIndexed(triangle);
		}

		// ---- Upsample, accumulating each level onto the one below ----

		RenderCommand::SetBlendMode(BlendMode::Additive);
		s_bloomData.UpsampleShader->Bind();

		for (size_t level = s_bloomData.Levels.size() - 1; level > 0; --level)
		{
			const FramebufferSpecification& sourceSpec = s_bloomData.Levels[level]->GetSpecification();

			uniformData.TexelSize = glm::vec2(1.0f / (float)sourceSpec.Width, 1.0f / (float)sourceSpec.Height);
			s_bloomData.SettingsUniformBuffer->SetData(&uniformData, sizeof(uniformData));

			s_bloomData.Levels[level]->BindColorAttachment(0, 0);

			s_bloomData.Levels[level - 1]->Bind();
			RenderCommand::DrawIndexed(triangle);
		}

		s_bloomData.Levels[0]->Unbind();

		RenderCommand::SetBlendMode(BlendMode::Alpha);
		RenderCommand::SetDepthTest(true);

		s_bloomData.HasResult = true;
	}

	void Bloom::BindResult(uint32_t slot)
	{
		if (s_bloomData.HasResult)
		{
			s_bloomData.Levels[0]->BindColorAttachment(0, slot);
		}
	}

	bool Bloom::HasResult()
	{
		return s_bloomData.HasResult;
	}

}
