#pragma once

#include "Uge/Core/Core.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include "Uge/Physics/Jolt/JoltPhysicsDebugRenderer.h"


namespace Uge
{


    /**
     * @brief Process-wide Jolt state owned by Uge::JoltAPI.
     *
     * Internal to the `Physics/Jolt/` folder: it names Jolt types, so no header outside
     * that folder may include it. Reach it through GetJoltData().
     */
    struct JoltData
    {
        /// Linear scratch allocator reused by every PhysicsSystem::Update.
        Scope<JPH::TempAllocatorImpl> TempAllocator;
        /// Worker pool Jolt runs its simulation jobs on.
        Scope<JPH::JobSystemThreadPool> JobThreadPool;
#ifdef JPH_DEBUG_RENDERER
        /// Sole JPH::DebugRenderer for the process; JPH::DebugRenderer::sInstance is a singleton.
        Scope<JoltDebugRenderer> DebugRenderer;
#endif
    };


    /**
     * @brief Accesses the state created by Uge::JoltAPI::Init().
     * @return The live JoltData.
     * @pre Uge::Physics::Init() must have been called.
     *
     * @warning JPH::TempAllocatorImpl is a linear allocator and is neither thread-safe nor
     * reentrant. Every Uge::JoltScene shares this one instance, which is only sound because
     * Step() is main-thread and the worlds never step concurrently.
     */
    JoltData& GetJoltData();
}