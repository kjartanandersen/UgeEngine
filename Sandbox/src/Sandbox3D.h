/**
 * @file Sandbox3D.h
 * @brief Sample layer exercising the 3D cube renderer.
 */

#pragma once

#include "Uge.h"

/**
 * @brief Demonstrates Uge::Renderer3D and the perspective camera controller.
 */
class Sandbox3D : public Uge::Layer
{

public:
	/** @brief Constructs the layer and its perspective camera controller. */
	Sandbox3D();
	/** @brief Default destructor. */
	virtual ~Sandbox3D() = default;

	/** @brief Loads the sample's texture and shader. */
	virtual void OnAttach() override;
	/** @brief Releases the sample's resources. */
	virtual void OnDetach() override;

	/**
	 * @brief Updates the camera and draws the cubes.
	 * @param ts Frame delta time.
	 */
	virtual void OnUpdate(Uge::Timestep ts) override;
	/**
	 * @brief Forwards events to the camera controller.
	 * @param e Event to handle.
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