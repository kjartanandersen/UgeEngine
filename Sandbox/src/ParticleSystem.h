/**
 * @file ParticleSystem.h
 * @brief The Sandbox's 2D particle system.
 */

#pragma once

#include <Uge.h>


/**
 * @brief A particle's spawn settings.
 *
 * Each `*Variation` field is the range of a random offset added to its base value.
 */
struct ParticleProps
{
	glm::vec2 Position; ///< Spawn position.
	glm::vec2 Velocity;          ///< Base velocity.
	glm::vec2 VelocityVariation; ///< Random range added to #Velocity.
	glm::vec4 ColorBegin;        ///< Colour at spawn.
	glm::vec4 ColorEnd;          ///< Colour at death.
	float SizeBegin;             ///< Size at spawn.
	float SizeEnd;               ///< Size at death.
	float SizeVariation;         ///< Random range added to #SizeBegin.
	float LifeTime = 5.0f; ///< Lifetime in seconds.
};

/**
 * @brief Emits, updates and draws 2D particles from a fixed pool. When the pool is full,
 * a new particle replaces a live one.
 */
class ParticleSystem
{
public:
	/**
	 * @brief Allocates the pool.
	 * @param maxParticles Pool size.
	 */
	ParticleSystem(uint32_t maxParticles = 100000);

	/**
	 * @brief Moves and ages the particles, and retires expired ones.
	 * @param ts Frame time.
	 */
	void OnUpdate(Uge::Timestep ts);
	/**
	 * @brief Draws the particles as rotated quads, blending colour and size over their life.
	 * @param camera The camera.
	 */
	void OnRender(Uge::OrthographicCamera& camera);

	/**
	 * @brief Emits a particle.
	 * @param particleProps Spawn settings.
	 */
	void Emit(const ParticleProps& particleProps);
private:
	struct Particle
	{
		glm::vec2 Position;
		/** @brief Velocity. */
		glm::vec2 Velocity; ///< Velocity.
		/** @brief Colour at spawn and at death. */
		glm::vec4 ColorBegin, ColorEnd;
		float Rotation = 0.0f; ///< Rotation in radians.
		/** @brief Size at spawn and at death. */
		float SizeBegin, SizeEnd; ///< Size at spawn and at death.

		float LifeTime = 1.0f; ///< Lifetime in seconds.
		float LifeRemaining = 0.0f; ///< Seconds left.

		bool Active = false; ///< Whether the slot holds a live particle.
	};
	std::vector<Particle> m_ParticlePool;
	uint32_t m_PoolIndex;

};