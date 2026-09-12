/**
 * @file ParticleSystem.h
 * @brief A pooled 2D particle system used by the Sandbox sample.
 */

#pragma once

#include <Uge.h>


/** @brief One pool slot's live state. */
/**
 * @brief The spawn parameters of a single particle.
 *
 * The `*Variation` fields are randomized ranges: the emitted particle gets the base
 * value plus a random offset within the variation, so one description yields a varied
 * burst.
 */
struct ParticleProps
{
	glm::vec2 Position; ///< Spawn position.
	glm::vec2 Velocity;          ///< Base velocity at spawn.
	glm::vec2 VelocityVariation; ///< Randomized range added to #Velocity.
	glm::vec4 ColorBegin;        ///< Colour at spawn.
	glm::vec4 ColorEnd;          ///< Colour at death; interpolated towards over the lifetime.
	float SizeBegin;             ///< Size at spawn.
	float SizeEnd;               ///< Size at death.
	float SizeVariation;         ///< Randomized range added to #SizeBegin.
	float LifeTime = 5.0f; ///< How long the particle lives, in seconds.
};

/**
 * @brief Emits, ages and draws 2D particles from a fixed-size pool.
 *
 * The pool is allocated once and reused: emitting overwrites the oldest slot rather
 * than allocating, so the system never allocates during a frame. Once the pool is full,
 * new particles replace live ones.
 *
 * @code
 * ParticleProps props;
 * props.Position = { 0.0f, 0.0f };
 * props.ColorBegin = { 1.0f, 0.5f, 0.0f, 1.0f };
 * for (int i = 0; i < 5; i++)
 *     m_particleSystem.Emit(props);
 * @endcode
 */
class ParticleSystem
{
public:
	/**
	 * @brief Allocates the particle pool.
	 * @param maxParticles Pool capacity; emitting beyond it recycles live particles.
	 */
	ParticleSystem(uint32_t maxParticles = 100000);

	/**
	 * @brief Ages every live particle and retires those whose lifetime has elapsed.
	 * @param ts Frame delta time.
	 */
	void OnUpdate(Uge::Timestep ts);
	/**
	 * @brief Draws every live particle as a rotated, fading quad.
	 * @param camera Camera to render with.
	 */
	void OnRender(Uge::OrthographicCamera& camera);

	/**
	 * @brief Spawns one particle into the pool.
	 * @param particleProps Spawn parameters, with variation applied.
	 */
	void Emit(const ParticleProps& particleProps);
private:
	struct Particle
	{
		glm::vec2 Position;
		/** @brief Current velocity. */
		glm::vec2 Velocity; ///< Current velocity.
		/** @brief Colour at spawn and at death. */
		glm::vec4 ColorBegin, ColorEnd;
		float Rotation = 0.0f; ///< Current rotation, in radians.
		/** @brief Size at spawn and at death. */
		float SizeBegin, SizeEnd; ///< Size at spawn and at death.

		float LifeTime = 1.0f; ///< Total lifetime, in seconds.
		float LifeRemaining = 0.0f; ///< Seconds left before this slot is retired.

		bool Active = false; ///< Whether this pool slot currently holds a live particle.
	};
	std::vector<Particle> m_ParticlePool;
	uint32_t m_PoolIndex;

};