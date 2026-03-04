#include <ugpch.h>
#include "Scene.h"

#include "Uge/Scene/Components.h"
#include "Uge/Renderer/Renderer2D.h"

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

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		
		tag.Tag = name.empty() ? "Entity" : name;

		return entity;



	}

	void Scene::DestroyEntity(Entity entity)
	{

		m_registry.destroy(entity);


	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{

		// Update Scripts
		{

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
		}
		Renderer2D::EndScene();

	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{

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


	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{

		static_assert(DependentFalse<T>::value, "Unsupported component type");

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



}
