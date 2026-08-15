#include <ugpch.h>
#include "Scene.h"

#include "Uge/Scene/Components.h"
#include "Uge/Scene/ScriptableEntity.h"

#include "Uge/Renderer/Renderer2D.h"

#include "Uge/Scripting/ScriptEngine.h"

#include "Uge/Asset/AssetManager.h"

#include "Uge/Renderer/ColorSpace.h"

#include <glm/gtc/quaternion.hpp>

#include <type_traits>

#include "Entity.h"

namespace Uge
{
	template<typename T>
	struct DependentFalse : std::false_type {};

	namespace
	{
		// Finds the scene's sky light and hands its environment to the mesh pass. Clears the
		// environment when there is none, since Model holds it across frames and would
		// otherwise keep lighting with a sky light that has since been deleted.
		static void ApplySkyLight(entt::registry& registry)
		{
			auto skyLightView = registry.view<SkyLightComponent>();
			for (auto entity : skyLightView)
			{
				const SkyLightComponent& skyLight = skyLightView.get<SkyLightComponent>(entity);

				if (skyLight.Environment && AssetManager::IsAssetHandleValid(skyLight.Environment))
				{
					Model::SetEnvironment(
						AssetManager::GetAsset<Environment>(skyLight.Environment), skyLight.Intensity);
					return;
				}
			}

			Model::SetEnvironment(nullptr);
		}

		// Finds the scene's directional light and hands it to the mesh pass. Clears it when
		// there is none, for the same reason ApplySkyLight does.
		static void ApplyDirectionalLight(entt::registry& registry)
		{
			auto lightView = registry.view<TransformComponent, DirectionalLightComponent>();
			for (auto [entity, transform, light] : lightView.each())
			{
				// Shines along the entity's local -Z, matching the axis a camera looks down,
				// so the same rotation gizmo aims both. Position is irrelevant: the source is
				// infinitely distant, which is what makes the rays parallel.
				const glm::vec3 direction =
					glm::mat3(glm::toMat3(glm::quat(transform.Rotation))) * glm::vec3(0.0f, 0.0f, -1.0f);

				// Picked by eye in the property panel, so sRGB; the renderer works in linear.
				Model::SetDirectionalLight(direction, SrgbToLinear(light.Color) * light.Intensity);
				return;
			}

			Model::SetDirectionalLight(glm::vec3(0.0f), glm::vec3(0.0f));
		}

		// The sky surrounds the camera rather than sitting somewhere in the world, so the view
		// matrix keeps its rotation and loses its translation.
		static glm::mat4 SkyboxViewProjection(const glm::mat4& projection, const glm::mat4& view)
		{
			return projection * glm::mat4(glm::mat3(view));
		}
	}

	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_viewportWidth = other->m_viewportWidth;
		newScene->m_viewportHeight = other->m_viewportHeight;
		newScene->SetName(other->GetName());

		auto& srcSceneRegistry = other->m_registry;
		auto& dstSceneRegistry = newScene->m_registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entities in new scene
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		// Copy components (except IDComponent and TagComponent)
		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{

		Entity entity = { m_registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();

		tag.Tag = name.empty() ? "Entity" : name;
		m_entityMap[uuid] = entity;

		return entity;


	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{

		if (m_entityMap.find(uuid) != m_entityMap.end())
		{

			return { m_entityMap.at(uuid), this };

		}

		return {  };
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{

		auto view = m_registry.view<TagComponent>();

		for (auto entity : view)
		{
			const auto& tagComponent = view.get<TagComponent>(entity);
			if (name == tagComponent.Tag)
			{
				return Entity{ entity, this };
			}

		}

		return {};

	}

	void Scene::DestroyEntity(Entity entity)
	{

		// Read the handle before the entity goes away, release after — the scan in
		// ReleaseMeshIfUnused would otherwise still see this entity holding it.
		AssetHandle mesh = entity.HasComponent<MeshComponent>()
			? entity.GetComponent<MeshComponent>().Mesh
			: 0;

		m_entityMap.erase(entity.GetUUID());
		m_registry.destroy(entity);

		ReleaseMeshIfUnused(mesh);

	}

	void Scene::ReleaseMeshIfUnused(AssetHandle mesh)
	{

		if (!mesh || m_isRunning)
		{
			return;
		}

		auto view = m_registry.view<MeshComponent>();
		for (auto entity : view)
		{
			if (view.get<MeshComponent>(entity).Mesh == mesh)
			{
				return;
			}
		}

		AssetManager::DeleteAsset(mesh);

	}

	

	void Scene::OnRuntimeStart()
	{
		m_isRunning = true;

		// Physics — must exist before scripts are instantiated.
		{
			PhysicsSceneDesc desc;
			m_physicsScene = Physics::CreateScene(desc);

			auto view = m_registry.view<TransformComponent, RigidbodyComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				CreatePhysicsBody(entity);   // new private helper, see below
			}

			m_physicsScene->OptimizeBroadPhase();
		}

		// Scripting
		{
			ScriptEngine::OnRuntimeStart(this);
			// Instantiate all script entities

			auto view = m_registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				ScriptEngine::OnCreateEntity(entity);
			}
		}

	}

	void Scene::OnRuntimeStop()
	{
		m_isRunning = false;
		ScriptEngine::OnRuntimeStop();

	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{

		if (!m_isPaused || m_stepFrames-- > 0)
		{
			// Update Scripts

			// C# Entity OnUpdate
			{
				UG_PROFILE_SCOPE("Scene Scripts (C#)");
				auto view = m_registry.view<ScriptComponent>();
				for (auto e : view)
				{
					Entity entity = { e, this };
					ScriptEngine::OnUpdateEntity(entity, ts);
				}
			}

			{
				UG_PROFILE_SCOPE("Scene Scripts (native)");
				m_registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
					{
						// TODO: Move to Scene::OnScenePlay
						if (!nsc.Instance)
						{
							nsc.Instance = nsc.InstantiateScript();
							nsc.Instance->m_entity = Entity{ entity, this };
							nsc.Instance->OnCreate();
						}

						nsc.Instance->OnUpdate(ts);

					});
			}
		}
		

		OnRenderUpdateRuntime(ts);

	}

	void Scene::OnRenderUpdateRuntime(Timestep ts)
	{


		// Render Scene
		Camera* mainCam = nullptr;
		glm::mat4 mainTransform;
		{
			auto view = m_registry.view<TransformComponent, CameraComponent>();
			for (auto [entity, transform, camera] : view.each())
			{

				//auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{

					mainCam = &camera.Cam;
					mainTransform = transform.GetTransform();
					break;
				}

			}
		}

		if (mainCam)
		{
			const glm::mat4 view = glm::inverse(mainTransform);
			const glm::mat4 viewProjection = mainCam->GetProjection() * view;

			ApplySkyLight(m_registry);
			ApplyDirectionalLight(m_registry);
			Model::BeginScene(viewProjection, glm::vec3(mainTransform[3]));
			{
				auto meshView = m_registry.view<TransformComponent, MeshComponent>();
				for (auto [entity, transform, mesh] : meshView.each())
				{
					if (mesh.Mesh)
					{
						Ref<Model> model = AssetManager::GetAsset<Model>(mesh.Mesh);
						if (model)
						{
							model->Draw(transform.GetTransform(), (int)entity);
						}
					}
				}
			}

			Model::DrawSkybox(SkyboxViewProjection(mainCam->GetProjection(), view));

			Model::EndScene();

			Renderer2D::BeginScene(mainCam->GetProjection(), mainTransform);
			{
				UG_PROFILE_SCOPE("Scene Renderer Draw");
				{
					auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
					for (auto ent : group)
					{
						auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(ent);

						Renderer2D::DrawSprite(transform.GetTransform(), sprite);

					}

				}

				// Draw Text
				{
					auto textView = m_registry.view<TransformComponent, TextComponent>();
					for (auto [entity, transform, text] : textView.each())
					{
						Renderer2D::DrawString(text.TextString, transform.GetTransform(), text);

					}

				}

				Renderer2D::EndScene();

			}
		}

	}

	void Scene::CreatePhysicsBody(Entity entity)
	{



	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{

		// Draw Meshes
		ApplySkyLight(m_registry);
		ApplyDirectionalLight(m_registry);
		Model::BeginScene(camera.GetViewProjection(), camera.GetPosition());
		{
			auto meshView = m_registry.view<TransformComponent, MeshComponent>();
			for (auto [entity, transform, mesh] : meshView.each())
			{
				if (mesh.Mesh)
				{
					Ref<Model> model = AssetManager::GetAsset<Model>(mesh.Mesh);
					if (model)
					{
						model->Draw(transform.GetTransform(), (int)entity);
					}
				}
			}
		}

		// After the opaque meshes so the depth buffer rejects sky the car already covers, and
		// before EndScene so transparent surfaces blend over it.
		Model::DrawSkybox(SkyboxViewProjection(camera.GetProjection(), camera.GetViewMatrix()));

		Model::EndScene();

		Renderer2D::BeginScene(camera);
		{
			UG_PROFILE_SCOPE("Scene Renderer Draw");

			// Draw Sprites
			{
				auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
				for (auto ent : group)
				{
					auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(ent);

					Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)ent);

				}
			}

			// Draw Text
			{
				auto textView = m_registry.view<TransformComponent, TextComponent>();
				for (auto [entity, transform, text] : textView.each())
				{
					Renderer2D::DrawString(text.TextString, transform.GetTransform(), text, (int)entity);

				}

			}
			
			
		
		}
		Renderer2D::EndScene();

	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (m_viewportWidth == width && m_viewportHeight == height)
		{
			return;
		}

		m_viewportWidth = width;
		m_viewportHeight = height;

		{
			auto view = m_registry.view<CameraComponent>();
			for (auto [entity, camComp] : view.each())
			{

				if (!camComp.FixedAspectRatio)
				{

					camComp.Cam.SetViewportSize(width, height);

				}


			}


		}


	}

	Entity Scene::GetPrimaryCameraEntity()
	{

		auto view = m_registry.view<CameraComponent>();

		for (auto camEntity : view)
		{
			const auto& camera = view.get<CameraComponent>(camEntity);
			if (camera.Primary)
			{
				return Entity{ camEntity, this };
			}
		
		}

		return {};
	}

	void Scene::Step(int frames)
	{

		m_stepFrames = frames;

	}

	Entity Scene::DuplicateEntity(Entity entity)
	{

		std::string name = entity.GetName();
		Entity newEnt = CreateEntity(name);
		CopyComponentIfExists(AllComponents{}, newEnt, entity);
		return newEnt;

	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{

		static_assert(DependentFalse<T>::value, "Unsupported component type");

	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{

		if (m_viewportWidth > 0 && m_viewportHeight > 0)
		{
			component.Cam.SetViewportSize(m_viewportWidth, m_viewportHeight);
		}

	}

	template<>
	void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<MeshComponent>(Entity entity, MeshComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TextComponent>(Entity entity, TextComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<SkyLightComponent>(Entity entity, SkyLightComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<DirectionalLightComponent>(Entity entity, DirectionalLightComponent& component)
	{
		// A light shines along its entity's local -Z, so an untouched transform points it
		// horizontally - which lights none of the upward-facing surfaces anyone is looking at,
		// and reads as "the light does nothing". Aim it down and to one side instead, the angle
		// a sun would actually come from.
		//
		// Only when the rotation is untouched, so this never overwrites a deliberate one.
		TransformComponent& transform = entity.GetComponent<TransformComponent>();
		if (transform.Rotation == glm::vec3(0.0f))
		{
			transform.Rotation = glm::vec3(glm::radians(-50.0f), glm::radians(-30.0f), 0.0f);
		}
	}

	template<>
	void Scene::OnComponentAdded<RigidbodyComponent>(Entity entity, RigidbodyComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& component)
	{
	}






}


