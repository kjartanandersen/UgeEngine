#include <ugpch.h>
#include "Scene.h"

#include "Uge/Scene/Components.h"
#include "Uge/Scene/ScriptableEntity.h"
#include "Uge/Renderer/Renderer2D.h"
#include "Uge/Scripting/ScriptEngine.h"

#include <type_traits>

#include "Entity.h"

namespace Uge
{
	template<typename T>
	struct DependentFalse : std::false_type {};


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

		m_entityMap.erase(entity.GetUUID());
		m_registry.destroy(entity);


	}

	void Scene::OnRuntimeStart()
	{
		m_isRunning = true;

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
			auto view = m_registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				ScriptEngine::OnUpdateEntity(entity, ts);
			}

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
			const glm::mat4 viewProjection = mainCam->GetProjection() * glm::inverse(mainTransform);

			Model::BeginScene(viewProjection);
			{
				auto meshView = m_registry.view<TransformComponent, MeshComponent>();
				for (auto [entity, transform, mesh] : meshView.each())
				{
					if (!mesh.ModelAsset)
					{
						continue;
					}

					mesh.ModelAsset->Draw(transform.GetTransform(), (int)entity);
				}
			}
			Model::EndScene();

			Renderer2D::BeginScene(mainCam->GetProjection(), mainTransform);
			{
				UG_PROFILE_SCOPE("Scene Renderer Draw");
				auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
				for (auto ent : group)
				{
					auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(ent);

					Renderer2D::DrawSprite(transform.GetTransform(), sprite);

				}
				Renderer2D::EndScene();

			}
		}

	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		Model::BeginScene(camera.GetViewProjection());
		{
			auto meshView = m_registry.view<TransformComponent, MeshComponent>();
			for (auto [entity, transform, mesh] : meshView.each())
			{
				if (!mesh.ModelAsset)
				{
					continue;
				}

				mesh.ModelAsset->Draw(transform.GetTransform(), (int)entity);
			}
		}
		Model::EndScene();

		Renderer2D::BeginScene(camera);
		{
			UG_PROFILE_SCOPE("Scene Renderer Draw");
			auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto ent : group)
			{
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(ent);

				Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)ent);

			}
			
			Renderer2D::DrawString(R"(
			// MSDF text shader

			#type vertex
			#version 450 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;
			layout(location = 2) in vec2 a_TexCoord;
			layout(location = 3) in int a_EntityID;

			layout(std140, binding = 0) uniform Camera
			{
				mat4 u_ViewProjection;
			};

			struct VertexOutput
			{
				vec4 Color;
				vec2 TexCoord;
			};

			layout (location = 0) out VertexOutput Output;
			layout (location = 2) out flat int v_EntityID;

			void main()
			{
				Output.Color = a_Color;
				Output.TexCoord = a_TexCoord;
				v_EntityID = a_EntityID;

				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
			}

			#type fragment
			#version 450 core

			layout(location = 0) out vec4 o_Color;
			layout(location = 1) out int o_EntityID;

			struct VertexOutput
			{
				vec4 Color;
				vec2 TexCoord;
			};

			layout (location = 0) in VertexOutput Input;
			layout (location = 2) in flat int v_EntityID;

			layout (binding = 0) uniform sampler2D u_FontAtlas;

			float screenPxRange() {
				const float pxRange = 2.0; // set to distance field's pixel range
				vec2 unitRange = vec2(pxRange)/vec2(textureSize(u_FontAtlas, 0));
				vec2 screenTexSize = vec2(1.0)/fwidth(Input.TexCoord);
				return max(0.5*dot(unitRange, screenTexSize), 1.0);
			}

			float median(float r, float g, float b) {
				return max(min(r, g), min(max(r, g), b));
			}

			void main()
			{
				vec4 texColor = Input.Color * texture(u_FontAtlas, Input.TexCoord);

				vec3 msd = texture(u_FontAtlas, Input.TexCoord).rgb;
				float sd = median(msd.r, msd.g, msd.b);
				float screenPxDistance = screenPxRange()*(sd - 0.5);
				float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
				if (opacity == 0.0)
					discard;

				vec4 bgColor = vec4(0.0);
				o_Color = mix(bgColor, Input.Color, opacity);
				if (o_Color.a == 0.0)
					discard;
	
				o_EntityID = v_EntityID;
			}
			)"
			, Font::GetDefault(), glm::mat4(1.0f), glm::vec4(1.0f));
		
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




}


