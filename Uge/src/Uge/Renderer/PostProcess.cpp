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

			// std140 rounds the block up to 16 bytes.
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

		// Binding 5; 0 to 4 are used by Renderer2D and Model.
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

		// Zero without a bloom result, so the shader can always sample slot 1.
		uniformData.BloomIntensity = Bloom::HasResult() ? s_settings.BloomIntensity : 0.0f;

		s_resolveData.SettingsUniformBuffer->SetData(&uniformData, sizeof(uniformData));

		target->Bind();

		s_resolveData.TonemapShader->Bind();
		source->BindColorAttachment(0, 0);
		Bloom::BindResult(1);

		// The target's depth buffer could reject the triangle.
		RenderCommand::SetDepthTest(false);
		RenderCommand::DrawIndexed(GetFullscreenTriangle());
		RenderCommand::SetDepthTest(true);
	}

	void PostProcess::Shutdown()
	{
		
		s_resolveData = ResolveData();
		
	}

}
