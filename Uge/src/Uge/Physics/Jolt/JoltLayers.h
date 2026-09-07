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
        /**
         * @brief Whether two object layers may generate contacts.
         * @param a First object layer.
         * @param b Second object layer.
         * @return `true` unless both layers are static.
         */
        bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override
        {
            return a == Moving || b == Moving;
        }
    };

    /**
     * @brief Maps every Uge::PhysicsLayer onto a Jolt broad-phase layer.
     *
     * Two broad-phase layers are enough: static geometry and everything that moves.
     * Keeping them apart lets Jolt skip the static-vs-static half of the tree entirely.
     */
    class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        /** @brief Builds the object-layer to broad-phase-layer table. */
        BroadPhaseLayerInterfaceImpl()
        {
            m_objectToBroadPhase[Static] = BroadPhase::Static;
            m_objectToBroadPhase[Moving] = BroadPhase::Moving;
        }

        /** @brief How many broad-phase layers exist. @return JoltLayers::BroadPhase::Count. */
        uint32_t GetNumBroadPhaseLayers() const override { return BroadPhase::Count; }

        /**
         * @brief The broad-phase layer an object layer belongs to.
         * @param layer Object layer to map; must be below JoltLayers::Count.
         * @return The matching broad-phase layer.
         */
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

    /**
     * @brief Decides which broad-phase layers an object layer is tested against.
     *
     * A static object only needs testing against the moving broad-phase layer; anything
     * that moves is tested against both.
     */
    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        /**
         * @brief Whether an object layer should be tested against a broad-phase layer.
         * @param layer Object layer of the body or query.
         * @param bpLayer Broad-phase layer being considered.
         * @return `true` if the pair may collide.
         */
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
        /**
         * @brief Builds a filter from a layer mask.
         * @param mask Bitmask of Uge::PhysicsLayer values to accept.
         */
        explicit MaskObjectLayerFilter(PhysicsLayerMask mask) : m_mask(mask) {}

        /**
         * @brief Whether a body's object layer passes the mask.
         * @param layer Object layer to test.
         * @return `true` if that layer's bit is set in the mask.
         */
        bool ShouldCollide(JPH::ObjectLayer layer) const override
        {
            return (m_mask & (1u << layer)) != 0;
        }

    private:
        PhysicsLayerMask m_mask;
    };

}