#include "ugpch.h"
#include "Physics.h"

#include "Uge/Physics/PhysicsScene.h"
#include "Uge/Physics/Jolt/JoltAPI.h"

namespace Uge
{
	Scope<PhysicsAPI> Physics::s_physicsAPI = nullptr;


	static Scope<PhysicsAPI> InitPhysicsAPI()
	{
		switch (PhysicsAPI::Current())
		{
		case PhysicsAPIType::None:
			UG_CORE_ASSERT(false, "PhysicsAPIType::None is not supported!");
			return nullptr;
		case PhysicsAPIType::Jolt:
			return CreateScope<JoltAPI>();
			break;
		default:
			return nullptr;
		}

		return nullptr;

	}

	void Physics::Init()
	{
		UG_CORE_ASSERT(!s_physicsAPI, "Physics is already initialized!");


		s_physicsAPI = InitPhysicsAPI();

		UG_CORE_ASSERT(s_physicsAPI, "Unknown PhysicsAPIType!");


		s_physicsAPI->Init();


	}

	void Physics::ShutDown()
	{

		if (!s_physicsAPI)
		{
			return;
		}

		s_physicsAPI->Shutdown();
		s_physicsAPI.reset();

	}
	Scope<PhysicsScene> Physics::CreateScene(const PhysicsSceneDesc& desc)
	{

		UG_CORE_ASSERT(s_physicsAPI, "Physics::Init has not been called!");
		return s_physicsAPI->CreateScene(desc);

	}
}

