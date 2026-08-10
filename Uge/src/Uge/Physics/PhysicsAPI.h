#pragma once

namespace Uge
{

	class PhysicsScene;
	struct PhysicsSceneDesc;

	/**
	* @brief Enum representing the library API used for physics
	* @ingroup group_physics
	* 
	* 
	*
	**/
	enum class PhysicsAPIType
	{
		None,
		Jolt

	};

	/**
	* @brief Abstract class for the physics API
	* @ingroup group_physics
	* 
	**/
	class PhysicsAPI
	{
	public:

		/**
			@brief PhysicsAPI object destructor
		**/
		virtual ~PhysicsAPI() = default;

		/**
			@brief Abstract function for initializing the physics API
		**/
		virtual void Init() = 0;

		/**
			@brief Abstract function for shutting down the physics API
		**/
		virtual void Shutdown() = 0;

		/**
			@brief  
			@param  desc - 
			@retval      - 
		**/
		virtual Scope<PhysicsScene> CreateScene(const PhysicsSceneDesc& desc) = 0;


		static PhysicsAPIType Current() { return s_currentPhysicsAPI; }
	private:
		inline static PhysicsAPIType s_currentPhysicsAPI = PhysicsAPIType::Jolt;

	};

}