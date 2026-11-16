/**
 * @file Sandbox2D.h
 * @brief The Sandbox's 2D sample layer.
 */

#pragma once

#include "Uge.h"
#include "ParticleSystem.h"

/**
 * @brief Draws quads, particles and a tile map with Uge::Renderer2D.
 */
class Sandbox2D : public Uge::Layer
{

public:
	/** @brief Creates the layer and its camera controller. */
	Sandbox2D();
	/** @brief Default destructor. */
	virtual ~Sandbox2D() = default;

	/**
	 * @brief Loads the font and sets up the atlas regions, tile map and particle settings.
	 * The sprite sheet's load is commented out.
	 */
	virtual void OnAttach() override;
	/** @brief Does nothing. */
	virtual void OnDetach() override;

	/**
	 * @brief Updates the camera and particles and draws the scene.
	 * @param ts Frame time.
	 */
	virtual void OnUpdate(Uge::Timestep ts) override;
	/**
	 * @brief Passes events to the camera controller.
	 * @param e The event.
	 */
	virtual void OnEvent(Uge::Event& e) override;
	/** @brief Draws the settings window and render counters. */
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