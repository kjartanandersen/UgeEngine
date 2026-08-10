#include "ugpch.h"
#include "JoltAPI.h"

#include "Uge/Physics/Jolt/JoltScene.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

namespace Uge
{

	struct JoltData
	{
		Scope<JPH::TempAllocatorImpl> TempAllocator;
		Scope<JPH::JobSystemThreadPool> JobThreadPool;


	};

	// Callback for traces, connect this to your own trace function if you have one
	static void TraceImpl(const char* inFMT, ...)
	{
		// Format the message

		va_list list;
		va_start(list, inFMT);
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), inFMT, list);
		va_end(list);

		// Print to the TTY
		UG_CORE_TRACE("[Jolt] {0}", buffer);
	}

#ifdef JPH_ENABLE_ASSERTS
	// Callback for asserts, connect this to your own assert handler if you have one
	static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint32_t inLine)
	{
		// Print to the TTY
		// std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << std::endl;
		UG_CORE_ERROR("[Jolt] {0}:{1}: ({2}) {3}", inFile, inLine, inExpression, (inMessage != nullptr ? inMessage : ""));

		// Breakpoint
		return true;
	};
#endif

	static JoltData* s_joltData = nullptr;


	JoltAPI::JoltAPI()
	{
	}
	JoltAPI::~JoltAPI()
	{

		if (!s_joltData)
			return;

		s_joltData->JobThreadPool.reset();
		s_joltData->TempAllocator.reset();

		delete s_joltData;
		s_joltData = nullptr;

		JPH::UnregisterTypes();

		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;

	}

	void JoltAPI::Init()
	{
		UG_CORE_ASSERT(!s_joltData, "Jolt can only be initialized once!");

		// Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
		// This needs to be done before any other Jolt function is called.
		JPH::RegisterDefaultAllocator();

		// Install trace and assert callbacks
		JPH::Trace = TraceImpl;

#ifdef JPH_ENABLE_ASSERTS
		JPH::AssertFailed = AssertFailedImpl;
#endif
		// Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
		// It is not directly used in this example but still required.
		JPH::Factory::sInstance = new JPH::Factory();

		// Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
		// If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
		// If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
		JPH::RegisterTypes();

		s_joltData = new JoltData();

		// We need a temp allocator for temporary allocations during the physics update.We're
		// pre-allocating 10 MB to avoid having to do allocations during the physics update.
		// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
		// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
		// malloc / free.
		s_joltData->TempAllocator = CreateScope<JPH::TempAllocatorImpl>(32 * 1024 * 1024);

		// We need a job system that will execute physics jobs on multiple threads. Typically
		// you would implement the JobSystem interface yourself and let Jolt Physics run on top
		// of your own job scheduler. JobSystemThreadPool is an example implementation.
		s_joltData->JobThreadPool = CreateScope<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::max(1u, std::thread::hardware_concurrency() - 1));



	}

	void JoltAPI::Shutdown()
	{


	}

	const JoltData* JoltAPI::GetJoltData()
	{
		return s_joltData;
	}

	Scope<PhysicsScene> JoltAPI::CreateScene(const PhysicsSceneDesc& desc)
	{
		UG_CORE_ASSERT(s_joltData, "JoltAPI::Init() has not been called!");
		return CreateScope<JoltScene>(desc);
	}


}