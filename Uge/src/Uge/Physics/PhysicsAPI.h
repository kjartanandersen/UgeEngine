#pragma once

namespace Uge
{

	class PhysicsScene;
	struct PhysicsSceneDesc;

	/**
	 * @brief Physics libraries.
	 * @ingroup group_physics
	 */
	enum class PhysicsAPIType
	{
		None,
		Jolt

	};

	/**
	 * @brief A physics backend.
	 * @ingroup group_physics
	 */
	class PhysicsAPI
	{
	public:

		/** @brief Virtual destructor. */
		virtual ~PhysicsAPI() = default;

		/** @brief Starts the backend. */
		virtual void Init() = 0;

		/** @brief Shuts the backend down. */
		virtual void Shutdown() = 0;

		/**
		 * @brief Creates a physics world.
		 * @param desc Limits and gravity.
		 * @return The world.
		 */
		virtual Scope<PhysicsScene> CreateScene(const PhysicsSceneDesc& desc) = 0;


		/**
		 * @brief The backend this build uses.
		 * @return The backend.
		 */
		static PhysicsAPIType Current() { return s_currentPhysicsAPI; }
	private:
		inline static PhysicsAPIType s_currentPhysicsAPI = PhysicsAPIType::Jolt;

	};

}