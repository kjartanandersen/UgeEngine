/**
 * @file Sandbox3D.h
 * @brief The Sandbox's 3D sample layer.
 */

#pragma once

#include "Uge.h"

/**
 * @brief Draws cubes with Uge::Renderer3D.
 */
class Sandbox3D : public Uge::Layer
{

public:
	/** @brief Creates the layer and its camera controller. */
	Sandbox3D();
	/** @brief Default destructor. */
	virtual ~Sandbox3D() = default;

	/** @brief Loads the font. */
	virtual void OnAttach() override;
	/** @brief Does nothing. */
	virtual void OnDetach() override;

	/**
	 * @brief Updates the camera and draws the cubes.
	 * @param ts Frame time.
	 */
	virtual void OnUpdate(Uge::Timestep ts) override;
	/**
	 * @brief Passes events to the camera controller.
	 * @param e The event.
	 */
	virtual void OnEvent(Uge::Event& e) override;
	/** @brief Draws the settings window. */
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