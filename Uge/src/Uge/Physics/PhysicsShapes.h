#pragma once

#include "Uge/Physics/PhysicsTypes.h"

#include <variant>
#include <vector>

namespace Uge
{

    /** @brief A box, in the body's local axes. */
    struct BoxShapeDesc
    {
        glm::vec3 HalfExtents = { 0.5f, 0.5f, 0.5f }; ///< Half the size on each axis.
    };

    /** @brief A sphere. */
    struct SphereShapeDesc
    {
        float Radius = 0.5f; ///< Radius.
    };

    /** @brief A capsule along the body's local Y axis. */
    struct CapsuleShapeDesc
    {
        float Radius = 0.5f;     ///< Cap radius.
        float HalfHeight = 0.5f; ///< Half the length of the cylinder, without the caps.
    };

    /** @brief A shape built from triangles. */
    struct MeshShapeDesc
    {
        std::vector<glm::vec3> Vertices; ///< Positions in mesh-local space.
        std::vector<uint32_t>  Indices;  ///< Three per triangle, counter-clockwise.
        bool Convex = true;              ///< `true` builds a convex hull, `false` a triangle mesh.
    };

    /** @brief Any collider shape. */
    using ShapeDesc = std::variant<BoxShapeDesc, SphereShapeDesc, CapsuleShapeDesc, MeshShapeDesc>;

    /** @brief One of a body's colliders. */
    struct ColliderDesc
    {
        ShapeDesc Shape;                              ///< The shape.
        glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };      ///< Offset from the body's origin.
        PhysicsMaterial Material;                     ///< Surface properties.
        bool IsTrigger = false;                       ///< Reports overlaps without colliding.
    };

    /** @brief The settings for creating a body. */
    struct BodyDesc
    {
        BodyType Type = BodyType::Dynamic;                  ///< How it moves.
        PhysicsLayer Layer = PhysicsLayer::Moving;          ///< Collision layer.

        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };          ///< World position.
        glm::quat Rotation = { 1.0f, 0.0f, 0.0f, 0.0f };    ///< World rotation.
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };             ///< Applied to the shapes at creation only.

        std::vector<ColliderDesc> Colliders;                ///< At least one; several become a compound shape.

        float Mass = 1.0f;                                  ///< In kg; `0` derives it from volume and density.
        float LinearDamping = 0.05f;                        ///< Velocity lost per second.
        float AngularDamping = 0.05f;                       ///< Angular velocity lost per second.
        float GravityFactor = 1.0f;                         ///< Multiplies gravity; `0` makes the body float.
        bool FixedRotation = false;                         ///< Locks rotation.

        uint64_t UserData = 0;                              ///< Stored with the body; scenes use the entity UUID.
    };


}