/**
 * @file Sandbox2D.h
 * @brief Sample layer exercising the 2D renderer.
 */

#pragma once

#include "Uge.h"
#include "ParticleSystem.h"

/**
 * @brief Demonstrates Uge::Renderer2D: sprites, an atlas, particles and a tile map.
 *
 * A worked example of the layer pattern — how a client subclasses Uge::Layer, drives a
 * camera controller, and issues batched 2D draws.
 */
class Sandbox2D : public Uge::Layer
{

public:
	/** @brief Constructs the layer and its orthographic camera controller. */
	Sandbox2D();
	/** @brief Default destructor. */
	virtual ~Sandbox2D() = default;

	/** @brief Loads the textures and builds the sprite atlas regions and tile map. */
	virtual void OnAttach() override;
	/** @brief Releases the sample's resources. */
	virtual void OnDetach() override;

	/**
	 * @brief Updates the camera and particles, then draws the scene.
	 * @param ts Frame delta time.
	 */
	virtual void OnUpdate(Uge::Timestep ts) override;
	/**
	 * @brief Forwards events to the camera controller.
	 * @param e Event to handle.
	 */
	virtual void OnEvent(Uge::Event& e) override;
	/** @brief Draws the settings window with colour pickers and renderer statistics. */
	virtual void OnImGuiRender() override;

private:

	Uge::OrthographicCameraController m_cameraController;

	// TODO: Temp
	Uge::Ref<Uge::Shader> m_flatColorShader;
	Uge::Ref<Uge::VertexArray> m_squareVA;

	Uge::Ref<Uge::Texture2D> m_texture;
	Uge::Ref<Uge::Texture2D> m_spriteSheet;
	Uge::Ref<Uge::SubTexture2D> m_textureStairs, m_textureBarrel, m_textureDirt;


	ImFont* m_mainFont;

	glm::vec4 m_square1Color = { 1.0f, 0.1f, 0.1f, 1.0f };
	glm::vec4 m_square2Color = { 0.1f, 0.1f, 1.1f, 1.0f };

	ParticleSystem m_particleSystem;
	ParticleProps m_particle;


	uint32_t m_mapWidth, m_mapHeight;
	std::unordered_map<char, Uge::Ref<Uge::SubTexture2D>> m_textureMap;



};