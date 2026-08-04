#include <ugpch.h>
#include "PostProcess.h"

#include "Uge/Renderer/Bloom.h"
#include "Uge/Renderer/Buffer.h"
#include "Uge/Renderer/FullscreenTriangle.h"
#include "Uge/Renderer/RenderCommand.h"

namespace Uge
{

	namespace
	{
		struct TonemapUniformData
		{
			float Exposure;
			int32_t Mode;
			float BloomIntensity;

			// std140 rounds a block up to a multiple of 16 bytes. Binding a range smaller than
			// the block is invalid, so the padding is what makes the buffer legal rather than
			// merely tidy.
			float Padding;
		};
	}

	PostProcess::ResolveData PostProcess::s_resolveData;
	RenderSettings PostProcess::s_settings;

	void PostProcess::EnsureResources()
	{
		if (s_resolveData.Initialized)
		{
			return;
		}

		s_resolveData.TonemapShader = Shader::Create("assets/shaders/Tonemap.glsl");

		// Binding 5: 0 is Renderer2D's camera, 1 the model transform, 2 the material,
		// 3 the mesh pass camera, and 4 is reserved for scene lighting.
		s_resolveData.SettingsUniformBuffer =
			UniformBuffer::Create(sizeof(TonemapUniformData), 5);

		s_resolveData.Initialized = true;
	}

	void PostProcess::Resolve(const Ref<Framebuffer>& source, const Ref<Framebuffer>& target)
	{
		UG_PROFILE_FUNCTION();

		if (!source || !target)
		{
			return;
		}

		UG_CORE_ASSERT(source != target, "PostProcess::Resolve cannot read and write one framebuffer");

		EnsureResources();

		TonemapUniformData uniformData{};
		uniformData.Exposure = s_settings.Exposure;
		uniformData.Mode = static_cast<int32_t>(s_settings.Tonemap);

		// Zeroed when there is no bloom result, so the shader can sample slot 1 unconditionally
		// rather than needing a branch and a second shader variant.
		uniformData.BloomIntensity = Bloom::HasResult() ? s_settings.BloomIntensity : 0.0f;

		s_resolveData.SettingsUniformBuffer->SetData(&uniformData, sizeof(uniformData));

		target->Bind();

		s_resolveData.TonemapShader->Bind();
		source->BindColorAttachment(0, 0);
		Bloom::BindResult(1);

		// The triangle covers the target by construction, so there is nothing for a depth test
		// to decide - and leaving it on would let whatever depth the scene pass left behind
		// reject the resolve entirely.
		RenderCommand::SetDepthTest(false);
		RenderCommand::DrawIndexed(GetFullscreenTriangle());
		RenderCommand::SetDepthTest(true);
	}

}
