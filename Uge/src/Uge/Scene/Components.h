/**
 * @file Components.h
 * @brief Every component type an entity can carry.
 * @ingroup group_scene
 *
 * Components are plain data. Behaviour lives in the systems that iterate over them —
 * mainly Uge::Scene's update functions — not in the components themselves.
 *
 * @par Adding a component type
 * Five coordinated changes are needed, and skipping any of them fails quietly rather
 * than loudly:
 * 1. declare the struct here;
 * 2. add it to Uge::AllComponents, or copy and duplicate will drop it;
 * 3. specialize Uge::Scene::OnComponentAdded in `Scene.cpp`, or the link fails;
 * 4. handle it in Uge::SceneSerializer, or it is lost on save;
 * 5. expose it in Uge::SceneHierarchyPanel so it can be edited.
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
	 * @brief The entity's persistent unique identifier.
	 * @ingroup group_scene
	 *
	 * Added automatically by Uge::Scene::CreateEntity. Unlike the `entt::entity` handle,
	 * this survives serialization, so it is what cross-entity references store.
	 */
	struct IDComponent
	{
		UUID ID; ///< The entity's persistent identifier.
		/** @brief Constructs the component with a freshly generated identifier. */
		IDComponent() = default;
		/** @brief Copy constructor. */
		IDComponent(const IDComponent&) = default;

	};

	/**
	 * @brief The entity's human-readable name.
	 * @ingroup group_scene
	 *
	 * Added automatically by Uge::Scene::CreateEntity and shown in the hierarchy panel.
	 * Not required to be unique.
	 */
	struct TagComponent
	{

		std::string Tag; ///< Display name shown in the hierarchy panel.

		/** @brief Constructs the component with an empty name. */
		TagComponent() = default;
		/** @brief Copy constructor. */
		TagComponent(const TagComponent&) = default;
		/**
		 * @brief Constructs the component with a name.
		 * @param tag Display name.
		 */
		TagComponent(const std::string& tag)
			: Tag(tag) {}


	};

	/**
	 * @brief Marks the entity as rendering an imported 3D model.
	 * @ingroup group_scene
	 *
	 * Drawn through Uge::Model, in a pass separate from the 2D renderer.
	 */
	struct MeshComponent
	{
		AssetHandle Mesh = 0; ///< Handle of the Uge::Model to draw; `0` draws nothing.


	};

	/**
	 * @brief The entity's position, orientation and scale.
	 * @ingroup group_scene
	 *
	 * Stored decomposed rather than as a matrix, so the property panel can edit each part
	 * independently. GetTransform() composes them on demand.
	 *
	 * @warning #Rotation is in **radians**, though the editor displays degrees.
	 */
	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f }; ///< Position in world space.
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f }; ///< Euler angles in **radians**.
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f }; ///< Scale along each axis.



		/** @brief Constructs an identity transform. */
		TransformComponent() = default;
		/** @brief Copy constructor. */
		TransformComponent(const TransformComponent&) = default;
		/**
		 * @brief Constructs the component at a position, with no rotation and unit scale.
		 * @param translation Initial position.
		 */
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		/**
		 * @brief Composes the model matrix.
		 * @return `translation * rotation * scale`.
		 * @see Uge::Math::DecomposeTransform for the inverse operation.
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
	 * @brief Marks the entity as rendering a 2D textured quad.
	 * @ingroup group_scene
	 *
	 * Drawn by Uge::Renderer2D::DrawSprite. With #Texture left at `0`, the quad renders as
	 * a flat #Color.
	 */
	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f }; ///< Tint multiplied with the texture, or the flat colour when untextured.
		AssetHandle Texture = 0; ///< Handle of the Uge::Texture2D to sample; `0` for none.
		float TilingFactor = 1.0f; ///< How many times the texture repeats across the quad.

		/** @brief Constructs an untextured white sprite. */
		SpriteRendererComponent() = default;
		/** @brief Copy constructor. */
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		/**
		 * @brief Constructs an untextured sprite of a single colour.
		 * @param color RGBA colour.
		 */
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}

	};
	
	/**
	 * @brief Lets the entity act as a viewpoint the runtime can render through.
	 * @ingroup group_scene
	 *
	 * The view matrix comes from the entity's Uge::TransformComponent, so moving the entity
	 * moves the camera.
	 *
	 * @note Exactly one camera in a scene should be #Primary. If none is, nothing renders
	 * at runtime; if several are, the first one found wins.
	 */
	struct CameraComponent
	{

		SceneCamera Cam; ///< Projection settings for this viewpoint.
		bool Primary = true; ///< Whether the runtime renders through this camera. @todo Move to scene.
		bool FixedAspectRatio = false; ///< When `true`, viewport resizes do not change the aspect ratio.

		/** @brief Constructs the component with a default orthographic camera. */
		CameraComponent() = default;
		/** @brief Copy constructor. */
		CameraComponent(const CameraComponent&) = default;


	};

	/**
	 * @brief Binds a C# class to the entity.
	 * @ingroup group_scene
	 *
	 * Uge::ScriptEngine instantiates #ClassName from the project's script assembly at
	 * runtime start and drives its `OnCreate` and `OnUpdate`.
	 *
	 * @see Uge::ScriptEngine, group_scripting
	 */
	struct ScriptComponent
	{

		std::string ClassName; ///< Fully qualified C# class name, e.g. `Sandbox.Player`.
		

		/** @brief Constructs the component with no class bound. */
		ScriptComponent() = default;
		/** @brief Copy constructor. */
		ScriptComponent(const ScriptComponent&) = default;

	};

	// Forward Decleration
	class ScriptableEntity;

	/**
	 * @brief Binds a C++ Uge::ScriptableEntity subclass to the entity.
	 * @ingroup group_scene
	 *
	 * The native alternative to Uge::ScriptComponent: no Mono involvement, and no reload
	 * without recompiling. Bind a type with Bind():
	 *
	 * @code
	 * entity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
	 * @endcode
	 */
	struct NativeScriptComponent
	{

		ScriptableEntity* Instance = nullptr; ///< The live script instance; null until the scene starts running.



		ScriptableEntity*(*InstantiateScript)() = nullptr; ///< Factory creating the script instance. @see Bind
		void (*DestroyScript)(NativeScriptComponent*) = nullptr; ///< Deleter destroying the script instance. @see Bind


		/**
		 * @brief Registers the script type to instantiate for this entity.
		 * @tparam T A class deriving from Uge::ScriptableEntity, default-constructible.
		 *
		 * Stores factory and deleter function pointers rather than constructing anything; the
		 * instance is created when the scene starts running.
		 */
		template<typename T>
		void Bind()
		{

			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };

		}


	};

	/**
	 * @brief Renders a string in world space using an MSDF font.
	 * @ingroup group_scene
	 *
	 * Drawn by Uge::Renderer2D::DrawString, so the text stays sharp at any scale.
	 */
	struct TextComponent
	{
		std::string TextString; ///< The text to render, interpreted as UTF-8.
		float Kerning = 0.0f; ///< Extra spacing between glyphs, in world units.
		float LineSpacing = 0.0f; ///< Extra spacing between lines, in world units.
		glm::vec4 Color{ 1.0f }; ///< Text colour, RGBA.

		Ref<Font> Font = Font::GetDefault(); ///< Font supplying the glyph atlas; defaults to the built-in font.
		

	};

	/**
	 * @brief Lights the scene from a single direction, like a sun.
	 * @ingroup group_scene
	 *
	 * The direction comes from the entity's Uge::TransformComponent: the light shines along
	 * the entity's local -Z, the same convention Uge::CameraComponent looks along, so
	 * rotating the entity aims the light. Position is ignored — the source is infinitely far
	 * away, which is what makes the rays parallel.
	 *
	 * @note #Color is authored in sRGB, as picked in the property panel, and converted to
	 * linear before it reaches the shader. @see Uge::SrgbToLinear
	 *
	 * @note Only one directional light is used; if several exist, the first found wins. A
	 * scene with none has no direct lighting at all and is lit purely by its
	 * Uge::SkyLightComponent, if it has one.
	 */
	struct DirectionalLightComponent
	{
		glm::vec3 Color{ 1.0f, 1.0f, 1.0f }; ///< Light colour in sRGB.

		/**
		 * @brief Radiance multiplier.
		 *
		 * Not a `[0, 1]` brightness: the shading model divides diffuse albedo by pi, so a
		 * value of 1 leaves a white surface at roughly a third of full white. Sunlight
		 * belongs well above 1.
		 */
		float Intensity = 3.0f;

		/** @brief Constructs a white light at default intensity. */
		DirectionalLightComponent() = default;
		/** @brief Copy constructor. */
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
	};

	/**
	 * @brief Lights the scene from an environment map, and draws it as the sky.
	 * @ingroup group_scene
	 *
	 * Supplies the ambient half of the lighting model: diffuse irradiance and roughness-
	 * dependent reflections from all directions, rather than from a single light. Without one,
	 * meshes fall back to a flat ambient term and metals have nothing to reflect.
	 *
	 * @note The transform is ignored — the environment surrounds the scene, so the entity's
	 * position and rotation mean nothing. Only one sky light is used; if several exist, the
	 * first found wins.
	 *
	 * @see Uge::Environment
	 */
	struct SkyLightComponent
	{
		AssetHandle Environment = 0; ///< Handle of the Uge::Environment to light with; `0` disables.
		float Intensity = 1.0f; ///< Multiplier applied to the environment's contribution.

		/** @brief Constructs a sky light with no environment assigned. */
		SkyLightComponent() = default;
		/** @brief Copy constructor. */
		SkyLightComponent(const SkyLightComponent&) = default;
	};

/**
 * @brief Makes the entity a simulated rigid body.
 * @ingroup group_scene
 *
 * Needs at least one collider component on the same entity to have any shape. Only
 * takes effect in play mode; the editor scene is never simulated.
 */
	struct RigidbodyComponent
	{
		BodyType Type			= BodyType::Dynamic;    ///< How the simulation moves the body.
		PhysicsLayer Layer		= PhysicsLayer::Moving; ///< Collision category.
		float Mass				= 1.0f;					///< Mass in kg; `0` derives it from the colliders.
		float LinearDamping		= 0.05f;                ///< Velocity bleed-off per second.
		float AngularDamping	= 0.05f;				///< Angular velocity bleed-off per second.
		float GravityFactor		= 1.0f;					///< Multiplier on scene gravity.
		bool FixedRotation		= false;				///< Locks rotation entirely.

		PhysicsBodyID RuntimeBody;                  ///< Live body while playing. Not serialized.

		/** @brief Constructs a default dynamic body. */
		RigidbodyComponent() = default;

		/**
		 * @brief Copies the authored settings but **not** the live body handle.
		 * @param other Component to copy.
		 *
		 * Uge::Scene::Copy is how play mode starts. A defaulted copy constructor would hand
		 * the play-mode scene handles into a world it does not own, and the first
		 * SetLinearVelocity would write through a dangling body index.
		 */
		RigidbodyComponent(const RigidbodyComponent& other)
			: Type(other.Type), Layer(other.Layer), Mass(other.Mass),
			LinearDamping(other.LinearDamping), AngularDamping(other.AngularDamping),
			GravityFactor(other.GravityFactor), FixedRotation(other.FixedRotation)
		{
			// RuntimeBody deliberately left invalid.
		}
	};

	/** @brief An axis-aligned box collider. @ingroup group_scene */
	struct BoxColliderComponent
	{
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };      ///< Local offset from the entity origin.
		glm::vec3 HalfExtents = { 0.5f, 0.5f, 0.5f }; ///< Half-size on each local axis.
		PhysicsMaterial Material;                     ///< Friction, restitution and density.
		bool IsTrigger = false;                       ///< Reports overlaps without colliding.
	};

	/** @brief A sphere collider. @ingroup group_scene */
	struct SphereColliderComponent
	{
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };		///< Local offset from the entity origin.
		float Radius = 0.5f;							///< 
		PhysicsMaterial Material;						///< 
		bool IsTrigger = false;							///< 
	};
	
	/** @brief A capsule collider. @ingroup group_scene */
	struct CapsuleColliderComponent
	{
		glm::vec3 Offset = { 0.0f, 0.0f, 0.0f };		///< Local offset from the entity origin.
		float Radius;									///< 
		float HalfHeight;								///< 
		PhysicsMaterial Material;						///< 
		bool IsTrigger = false;							///< 
	};

	/** @brief A mesh collider. @ingroup group_scene */
	struct MeshColliderComponent
	{
		AssetHandle Mesh = 0;							///< 
		bool Convex = true;								///< 
		PhysicsMaterial Material;						///< 
		bool IsTrigger = false;							///< 
	};

	/**
	 * @brief A compile-time list of component types.
	 * @tparam Component The types in the list.
	 * @ingroup group_scene
	 *
	 * Carries no data. It exists so scene copying and entity duplication can expand over
	 * every component type in one variadic operation instead of naming each type
	 * individually.
	 */
	template<typename... Component>
	struct ComponentGroup
	{
	};

	/**
	 * @brief Every copyable component type, used by scene copy and entity duplication.
	 * @ingroup group_scene
	 *
	 * @warning A new component type must be added here, or Uge::Scene::Copy and
	 * Uge::Scene::DuplicateEntity will silently drop it.
	 *
	 * @note Uge::IDComponent and Uge::TagComponent are deliberately absent; they are
	 * handled separately so a duplicate gets a fresh UUID.
	 */
	using AllComponents =
		ComponentGroup<TransformComponent, SpriteRendererComponent,
		 CameraComponent, ScriptComponent, MeshComponent,
		NativeScriptComponent, TextComponent, SkyLightComponent,
		DirectionalLightComponent, RigidbodyComponent, 
		BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent, 
		MeshColliderComponent>;


}




