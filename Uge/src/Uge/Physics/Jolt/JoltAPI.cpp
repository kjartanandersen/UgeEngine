#include "ugpch.h"
#include "JoltAPI.h"

#include "Uge/Physics/Jolt/JoltScene.h"
#include "Uge/Physics/Jolt/JoltData.h"

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

	// Sends Jolt's trace output to the engine log.
	static void TraceImpl(const char* inFMT, ...)
	{

		va_list list;
		va_start(list, inFMT);
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), inFMT, list);
		va_end(list);

		UG_CORE_TRACE("[Jolt] {0}", buffer);
	}

#ifdef JPH_ENABLE_ASSERTS
	// Logs a failed Jolt assert. Returning true breaks into the debugger.
	static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint32_t inLine)
	{
		// std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << std::endl;
		UG_CORE_ERROR("[Jolt] {0}:{1}: ({2}) {3}", inFile, inLine, inExpression, (inMessage != nullptr ? inMessage : ""));

		return true;
	};
#endif

	static JoltData* s_joltData = nullptr;
	
	JoltAPI::JoltAPI()
	{
	}

	JoltAPI::~JoltAPI()
	{
		Shutdown();
		

	}

	void JoltAPI::Init()
	{
		UG_CORE_ASSERT(!s_joltData, "Jolt can only be initialized once!");

		// Jolt uses malloc and free. This must come before any other Jolt call.
		JPH::RegisterDefaultAllocator();

		JPH::Trace = TraceImpl;

#ifdef JPH_ENABLE_ASSERTS
		JPH::AssertFailed = AssertFailedImpl;
#endif
		// Jolt requires a factory, though the engine doesn't use it.
		JPH::Factory::sInstance = new JPH::Factory();

		// Registers the physics types and their collision handlers.
		JPH::RegisterTypes();

		s_joltData = new JoltData();

		// 32 MB of scratch memory, allocated up front so a step never allocates.
		s_joltData->TempAllocator = CreateScope<JPH::TempAllocatorImpl>(32 * 1024 * 1024);

		// Physics jobs run on every core but one.
		s_joltData->JobThreadPool = CreateScope<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::max(1, (int)std::thread::hardware_concurrency() - 1));

#ifdef JPH_DEBUG_RENDERER
		s_joltData->DebugRenderer = CreateScope<JoltDebugRenderer>();
#endif

	}

	void JoltAPI::Shutdown()
	{
		if (!s_joltData)
			return;

#ifdef JPH_DEBUG_RENDERER
		s_joltData->DebugRenderer.reset();
#endif

		s_joltData->JobThreadPool.reset();
		s_joltData->TempAllocator.reset();

		delete s_joltData;
		s_joltData = nullptr;

		JPH::UnregisterTypes();

		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;

	}

	Scope<PhysicsScene> JoltAPI::CreateScene(const PhysicsSceneDesc& desc)
	{
		UG_CORE_ASSERT(s_joltData, "JoltAPI::Init() has not been called!");
		return CreateScope<JoltScene>(desc);
	}

	JoltData& GetJoltData()
	{
		UG_CORE_VERIFY(s_joltData, "Physics::Init() has not been called!");
		return *s_joltData;
	}

}