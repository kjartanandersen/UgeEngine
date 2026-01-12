#pragma once

#include "Uge.h"

class Sandbox3D : public Uge::Layer
{

public:
	Sandbox3D();
	virtual ~Sandbox3D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Uge::Timestep ts) override;
	virtual void OnEvent(Uge::Event& e) override;
	virtual void OnImGuiRender() override;

private:

	Uge::PerspectiveCameraController m_cameraController;

	// TODO: Temp
	Uge::Ref<Uge::Shader> m_flatColorShader;
	Uge::Ref<Uge::VertexArray> m_squareVA;
	Uge::Ref<Uge::Texture2D> m_texture;

	ImFont* m_mainFont;

	glm::vec4 m_square1Color = { 1.0f, 0.1f, 0.1f, 1.0f };
	glm::vec4 m_square2Color = { 0.1f, 0.1f, 1.1f, 1.0f };




};