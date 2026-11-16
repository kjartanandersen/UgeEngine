/**
 * @file Components.h
 * @brief The component types.
 * @ingroup group_scene
 *
 * A new component type also needs an entry in Uge::AllComponents, a
 * Uge::Scene::OnComponentAdded specialization, Uge::SceneSerializer support and UI in
 * Uge::SceneHierarchyPanel.
 */

#pragma once

#include "Uge/Core/Core.h"
#include "Uge/Core/UUID.h"
#include "Uge/Renderer/Model.h"
#include "Uge/Scene/SceneCamera.h"
#include "Uge/Renderer/Texture.h"
#include "Uge/Project/Project.h"
#include "Uge/Renderer/Font.h"
#include "Uge/Physics/PhysicsTypes.h"

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Uge
{

	/**
	 * @brief The entity's persistent ID. Unlike the `entt::entity`, it survives saving.
	 * @ingroup group_scene
	 */
	struct IDComponent
	{
		UUID ID; ///< The ID.
		/** @brief The null ID. */
		IDComponent() = default;
		/** @brief Copy constructor. */
		IDComponent(const IDComponent&) = default;

	};

	/**
	 * @brief The entity's name. Names need not be unique.
	 * @ingroup group_scene
	 */
	struct TagComponent
	{

		std::string Tag; ///< The name.

		/** @brief An empty name. */
		TagComponent() = default;
		/** @brief Copy constructor. */
		TagComponent(const TagComponent&) = default;
		/**
		 * @brief A component with a name.
		 * @param tag The name.
		 */
		TagComponent(const std::string& tag)
			: Tag(tag) {}


	};

	/**
	 * @brief Draws a 3D model.
	 * @ingroup group_scene
	 */
	struct MeshComponent
	{
		AssetHandle Mesh = 0; ///< The Uge::Model; `0` draws nothing.


	};

	/**
	 * @brief The entity's position, rotation and scale. Rotation is in radians.
	 * @ingroup group_scene
	 */
	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f }; ///< Position.
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f }; ///< Euler angles in radians.
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f }; ///< Scale on each axis.



		/** @brief An identity transform. */
		TransformComponent() = default;
		/** @brief Copy constructor. */
		TransformComponent(const TransformComponent&) = default;
		/**
		 * @brief A transform at a position.
		 * @param translation The position.
		 */
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		/**
		 * @brief The model matrix.
		 * @return `translation * rotation * scale`.
		 */
		glm::mat4 GetTransform() const
		{
			glm::mat4 translation = glm::translate(glm::mat4(1.0f), Translation);

			glm::mat4 rotation = glm::mat4_cast(glm::quat(Rotation));

			glm::mat4 scale = glm::scale(glm::mat4(1.0f), Scale);

			return translation * rotation * scale;
		}

	};
	
	/**
	 * @brief Draws a 2D quad, textured or in a flat colour.
	 * @ingroup group_scene
	 */
	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f }; ///< Tint, or the whole colour without a texture.
		AssetHandle Texture = 0; ///< The Uge::Texture2D; `0` for none.
		float TilingFactor = 1.0f; ///< Times the texture repeats across the quad.

		/** @brief An untextured white sprite. */
		SpriteRendererComponent() = default;
		/** @brief Copy constructor. */
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		/**
		 * @brief An untextured sprite.
		 * @param color RGBA colour.
		 */
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}

	};

	/**
	 * @brief A camera at the entity's transform.
	 * @ingroup group_scene
	 *
	 * The runtime renders through the first #Primary camera; with none, nothing renders.
	 */
	struct CameraComponent
	{

		SceneCamera Cam; ///< Projection settings.
		bool Primary = true; ///< Whether the runtime renders through it. @todo Move to the scene.
		bool FixedAspectRatio = false; ///< `true` keeps the aspect ratio when the viewport resizes.

		/** @brief A default orthographic camera. */
		CameraComponent() = default;
		/** @brief Copy constructor. */
		CameraComponent(const CameraComponent&) = default;


	};

	/**
	 * @brief Runs a C# class on the entity in play mode.
	 * @ingroup group_scene
	 */
	struct ScriptComponent
	{

		std::string ClassName; ///< Full C# class name, such as `Sandbox.Player`.


		/** @brief No class. */
		ScriptComponent() = default;
		/** @brief Copy constructor. */
		ScriptComponent(const ScriptComponent&) = default;

	};

	class ScriptableEntity;

	/**
	 * @brief Runs a C++ Uge::ScriptableEntity subclass on the entity.
	 * @ingroup group_scene
	 */
	struct NativeScriptComponent
	{

		ScriptableEntity* Instance = nullptr; ///< The script object; null until play starts.



		ScriptableEntity*(*InstantiateScript)() = nullptr; ///< Creates the script object.
		void (*DestroyScript)(NativeScriptComponent*) = nullptr; ///< Deletes the script object.


		/**
		 * @brief Sets the script type. The object is created when play starts.
		 * @tparam T A default-constructible Uge::ScriptableEntity subclass.
		 */
		template<typename T>
		void Bind()
		{

			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };

		}


	};

	/**
	 * @brief Draws text in world space with an MSDF font.
	 * @ingroup group_scene
	 */
	struct TextComponent
	{
		std::string TextString; ///< The text, in UTF-8.
		float Kerning = 0.0f; ///< Extra space between glyphs, in world units.
		float LineSpacing = 0.0f; ///< Extra space between lines, in world units.
		glm::vec4 Color{ 1.0f }; ///< RGBA colour.

		Ref<Font> Font = Font::GetDefault(); ///< The font; the built-in one by default.


	};

	/**
	 * @brief A light with parallel rays, like the sun.
	 * @ingroup group_scene
	 *
	 * It shines along the entity's local -Z; its position is ignored. Only the first
	 * directional light in a scene is used. #Color is sRGB and is converted to linear for the
	 * shader.
	 */
	struct DirectionalLightComponent
	{
		glm::vec3 Color{ 1.0f, 1.0f, 1.0f }; ///< Colour, in sRGB.

		/**
		 * @brief Radiance multiplier.
		 *
		 * Diffuse albedo is divided by pi, so at 1 a white surface reaches about a third of
		 * full white. Sunlight needs well above 1.
		 */
		float Intensity = 3.0f;

		/** @brief A white light at the default intensity. */
		DirectionalLightComponent() = default;
		/** @brief Copy constructor. */
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
	};

	/**
	 * @brief Lights the scene from an environment map, and draws it as the sky.
	 * @ingroup group_scene
	 *
	 * Without one, meshes get a flat ambient term. The transform is ignored, and only the
	 * first sky light in a scene is used.
	 */
	struct SkyLightComponent
	{
		AssetHandle Environment = 0; ///< The Uge::Environment; `0` turns it off.
		float Intensity = 1.0f; ///< Multiplies the environment's light.

		/** @brief A sky light without an environment. */
		SkyLightComponent() = default;
		/** @brief Copy constructor. */
		SkyLightComponent(const SkyLightComponent&) = default;
	};

	/**
	 * @brief Makes the entity a rigid body in play mode. It needs a collider to have a shape.
	 * @ingroup group_scene
	 */
	struct RigidbodyComponent
	{
		BodyType Type			= BodyType::Dynamic;    ///< How it moves.
		PhysicsLayer Layer		= PhysicsLayer::Moving; ///< Collision layer.
		float Mass				= 1.0f;					///< In kg; `0` derives it from the colliders.
		float LinearDamping		= 0.05f;                ///< Velocity lost per second.
		float AngularDamping	= 0.05f;				///< Angular velocity lost per second.
		float GravityFactor		= 1.0f;					///< Multiplies gravity.
		bool FixedRotation		= false;				///< Locks rotation.

		PhysicsBodyID RuntimeBody;                  ///< The body while playing. Not saved.

		/** @brief A dynamic body. */
		RigidbodyComponent() = default;

		/**
		 * @brief Copies the settings, but not the body handle.
		 * @param other Component to copy.
		 *
		 * Play mode starts from a copy of the scene, which has a world of its own.
		 */
		RigidbodyComponent(const RigidbodyComponent& other)
			: Type(other.Type), Layer(other.Layer), Mass(other.Mass),
			LinearDamping(other.LinearDamping), AngularDamping(other.AngularDamping),
			GravityFactor(other.GravityFactor), FixedRotation(other.FixedRotation)
		{
			// RuntimeBody stays invalid.
		}
	};

	/** @brief A box collider. @ingroup group_scene */
	struct BoxColliderComponent
	{
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };      ///< Offset from the entity's origin.
		glm::vec3 HalfExtents = { 0.5f, 0.5f, 0.5f }; ///< Half the size on each local axis.
		PhysicsMaterial Material;                     ///< Friction, restitution and density.
		bool IsTrigger = false;                       ///< Reports overlaps without colliding.
	};

	/** @brief A sphere collider. @ingroup group_scene */
	struct SphereColliderComponent
	{
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };		///< Offset from the entity's origin.
		float Radius = 0.5f;							///< Radius, before the entity's scale.
		PhysicsMaterial Material;						///< Friction, restitution and density.
		bool IsTrigger = false;							///< Reports overlaps without colliding.
	};

	/** @brief A capsule collider. @ingroup group_scene */
	struct CapsuleColliderComponent
	{
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };		///< Offset from the entity's origin.
		float Radius = 0.5f;							///< Cap radius.
		float HalfHeight = 0.5f;						///< Half the length of the cylinder, without the caps.
		PhysicsMaterial Material;						///< Friction, restitution and density.
		bool IsTrigger = false;							///< Reports overlaps without colliding.
	};

	/** @brief A mesh collider. @ingroup group_scene */
	struct MeshColliderComponent
	{
		AssetHandle Mesh = 0;							///< The Uge::Model to build the shape from; `0` gives no shape.
		bool Convex = true;								///< `true` builds a convex hull, `false` a triangle mesh.
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };		///< Offset from the entity's origin.
		PhysicsMaterial Material;						///< Friction, restitution and density.
		bool IsTrigger = false;							///< Reports overlaps without colliding.
	};

	/**
	 * @brief A list of component types, for expanding over all of them at once.
	 * @tparam Component The types.
	 * @ingroup group_scene
	 */
	template<typename... Component>
	struct ComponentGroup
	{
	};

	/**
	 * @brief The component types that scene copy and entity duplication copy.
	 * @ingroup group_scene
	 *
	 * A component type missing here is silently dropped by Uge::Scene::Copy and
	 * Uge::Scene::DuplicateEntity. Uge::IDComponent and Uge::TagComponent are copied
	 * separately, so a duplicate gets a new UUID.
	 */
	using AllComponents =
		ComponentGroup<TransformComponent, SpriteRendererComponent,
		 CameraComponent, ScriptComponent, MeshComponent,
		NativeScriptComponent, TextComponent, SkyLightComponent,
		DirectionalLightComponent, RigidbodyComponent, 
		BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent, 
		MeshColliderComponent>;


}




