#pragma once

#include "Uge/Physics/PhysicsTypes.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

namespace Uge::JoltLayers
{

    // Object layers map 1:1 onto Uge::PhysicsLayer.
    static constexpr JPH::ObjectLayer Static = (JPH::ObjectLayer)PhysicsLayer::Static;
    static constexpr JPH::ObjectLayer Moving = (JPH::ObjectLayer)PhysicsLayer::Moving;
    static constexpr JPH::ObjectLayer Count = (JPH::ObjectLayer)PhysicsLayer::Count;

    namespace BroadPhase
    {
        static constexpr JPH::BroadPhaseLayer Static(0);
        static constexpr JPH::BroadPhaseLayer Moving(1);
        static constexpr uint32_t Count = 2;
    }

    /// Static bodies never need to collide with each other.
    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override
        {
            return a == Moving || b == Moving;
        }
    };

    class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BroadPhaseLayerInterfaceImpl()
        {
            m_objectToBroadPhase[Static] = BroadPhase::Static;
            m_objectToBroadPhase[Moving] = BroadPhase::Moving;
        }

        uint32_t GetNumBroadPhaseLayers() const override { return BroadPhase::Count; }

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
        {
            JPH_ASSERT(layer < Count);
            return m_objectToBroadPhase[layer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
        {
            switch ((JPH::BroadPhaseLayer::Type)layer)
            {
            case (JPH::BroadPhaseLayer::Type)BroadPhase::Static: return "Static";
            case (JPH::BroadPhaseLayer::Type)BroadPhase::Moving: return "Moving";
            default: return "Unknown";
            }
        }
#endif

    private:
        JPH::BroadPhaseLayer m_objectToBroadPhase[Count];
    };

    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer bpLayer) const override
        {
            if (layer == Static) return bpLayer == BroadPhase::Moving;
            return true;
        }
    };

    /**
     * @brief Restricts a query to the layers named in a Uge::PhysicsLayerMask.
     *
     * Bit `N` of the mask enables Uge::PhysicsLayer `N`, so Uge::PhysicsLayerMaskAll
     * (`~0u`) passes everything.
     */
    class MaskObjectLayerFilter : public JPH::ObjectLayerFilter
    {
    public:
        explicit MaskObjectLayerFilter(PhysicsLayerMask mask) : m_mask(mask) {}

        bool ShouldCollide(JPH::ObjectLayer layer) const override
        {
            return (m_mask & (1u << layer)) != 0;
        }

    private:
        PhysicsLayerMask m_mask;
    };

}