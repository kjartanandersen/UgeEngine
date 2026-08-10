#pragma once

#include "Uge/Physics/PhysicsTypes.h"
#include "Uge/Asset/Asset.h"

#include <variant>
#include <vector>

namespace Uge
{

    /** @brief An axis-aligned box, before the body's rotation is applied. */
    struct BoxShapeDesc
    {
        glm::vec3 HalfExtents = { 0.5f, 0.5f, 0.5f }; ///< Half-size on each axis.
    };

    /** @brief A sphere. */
    struct SphereShapeDesc
    {
        float Radius = 0.5f; ///< Sphere radius.
    };

    /** @brief A capsule aligned to the body's local Y axis. */
    struct CapsuleShapeDesc
    {
        float Radius = 0.5f;     ///< Radius of the hemispherical caps.
        float HalfHeight = 0.5f; ///< Half the length of the cylindrical section only.
    };

    /** @brief A shape built from a Uge::Model asset. */
    struct MeshShapeDesc
    {
        AssetHandle Mesh = 0; ///< Model to build from; `0` produces no shape.
        bool Convex = true;   ///< `true` builds a convex hull, `false` a triangle mesh.
    };

    /** @brief Any shape a collider component can describe. */
    using ShapeDesc = std::variant<BoxShapeDesc, SphereShapeDesc, CapsuleShapeDesc, MeshShapeDesc>;

    /** @brief One collider attached to a body. */
    struct ColliderDesc
    {
        ShapeDesc Shape;                              ///< Geometry of this collider.
        glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };      ///< Local offset from the body origin.
        PhysicsMaterial Material;                     ///< Surface response.
        bool IsTrigger = false;                       ///< Reports overlaps without responding.
    };

    /**
     * @brief Everything needed to create one body.
     *
     * A plain struct rather than a builder or a class hierarchy, so adding a parameter
     * does not change any virtual signature.
     */
    struct BodyDesc
    {
        BodyType Type = BodyType::Dynamic;                  ///< Motion behaviour.
        PhysicsLayer Layer = PhysicsLayer::Moving;          ///< Collision category.

        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };          ///< Initial world position.
        glm::quat Rotation = { 1.0f, 0.0f, 0.0f, 0.0f };    ///< Initial world orientation.
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };             ///< Baked into the shapes. @see PhysicsScene

        std::vector<ColliderDesc> Colliders;                ///< One or more; several become a compound shape.

        float Mass = 1.0f;                                  ///< Body mass in kg; `0` derives it from volume and density.
        float LinearDamping = 0.05f;                        ///< Velocity bleed-off per second.
        float AngularDamping = 0.05f;                       ///< Angular velocity bleed-off per second.
        float GravityFactor = 1.0f;                         ///< Multiplier on scene gravity; `0` makes it float.
        bool FixedRotation = false;                         ///< Locks all rotation, for characters and top-down movers.

        uint64_t UserData = 0;                              ///< Round-tripped verbatim; Uge::Scene stores the entity UUID.
    };


}