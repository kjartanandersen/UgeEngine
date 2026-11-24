#pragma once

#include "Uge/Core/Core.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include "Uge/Physics/Jolt/JoltPhysicsDebugRenderer.h"


namespace Uge
{


    /**
     * @brief Jolt state owned by Uge::JoltAPI.
     *
     * It names Jolt types, so only files in `Physics/Jolt/` may include it.
     */
    struct JoltData
    {
        /// Scratch allocator for PhysicsSystem::Update.
        Scope<JPH::TempAllocatorImpl> TempAllocator;
        /// Worker threads for the simulation.
        Scope<JPH::JobSystemThreadPool> JobThreadPool;
#ifdef JPH_DEBUG_RENDERER
        /// The only JPH::DebugRenderer; Jolt allows one per process.
        Scope<JoltDebugRenderer> DebugRenderer;
#endif
    };


    /**
     * @brief The state created by Uge::JoltAPI::Init().
     * @return The state.
     * @pre Uge::Physics::Init() has run.
     *
     * Every scene shares the temp allocator, which isn't thread-safe, so scenes must not
     * step at the same time.
     */
    JoltData& GetJoltData();
}