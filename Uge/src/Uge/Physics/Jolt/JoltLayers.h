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

    /// Lets every pair of layers collide except two static ones.
    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
    {
    public:
        /**
         * @brief Whether two object layers can collide.
         * @param a First layer.
         * @param b Second layer.
         * @return `true` unless both are static.
         */
        bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override
        {
            return a == Moving || b == Moving;
        }
    };

    /**
     * @brief Maps the object layers onto two broad-phase layers, static and moving.
     *
     * Static bodies get their own layer, so static pairs are never tested.
     */
    class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        /** @brief Builds the layer table. */
        BroadPhaseLayerInterfaceImpl()
        {
            m_objectToBroadPhase[Static] = BroadPhase::Static;
            m_objectToBroadPhase[Moving] = BroadPhase::Moving;
        }

        /** @brief The number of broad-phase layers. @return JoltLayers::BroadPhase::Count. */
        uint32_t GetNumBroadPhaseLayers() const override { return BroadPhase::Count; }

        /**
         * @brief The broad-phase layer of an object layer.
         * @param layer Object layer, below JoltLayers::Count.
         * @return The broad-phase layer.
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
     * @brief Tests static objects against moving bodies only, and moving objects against both.
     */
    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        /**
         * @brief Whether an object layer is tested against a broad-phase layer.
         * @param layer Object layer.
         * @param bpLayer Broad-phase layer.
         * @return `true` if they can collide.
         */
        bool ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer bpLayer) const override
        {
            if (layer == Static) return bpLayer == BroadPhase::Moving;
            return true;
        }
    };

    /**
     * @brief Limits a query to the layers in a Uge::PhysicsLayerMask. Bit N enables layer N.
     */
    class MaskObjectLayerFilter : public JPH::ObjectLayerFilter
    {
    public:
        /**
         * @brief Creates the filter.
         * @param mask Layers to accept.
         */
        explicit MaskObjectLayerFilter(PhysicsLayerMask mask) : m_mask(mask) {}

        /**
         * @brief Whether a layer is in the mask.
         * @param layer Object layer.
         * @return `true` if its bit is set.
         */
        bool ShouldCollide(JPH::ObjectLayer layer) const override
        {
            return (m_mask & (1u << layer)) != 0;
        }

    private:
        PhysicsLayerMask m_mask;
    };

}