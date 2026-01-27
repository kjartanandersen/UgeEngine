#include <ugpch.h>
#include "Scene.h"

#include "Uge/Scene/Components.h"
#include "Uge/Renderer/Renderer2D.h"

#include "Entity.h"

namespace Uge
{

	static void OnTransformConstruct(entt::registry& registry, entt::entity entity)
	{

	}

	

	Scene::Scene()
	{

#if 0
		entt::entity entity = m_registry.create();
		m_registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));

		m_registry.on_construct<TransformComponent>().connect<&OnTransformConstruct>();
		if (m_registry.any_of<TransformComponent>(entity))
		{
			TransformComponent& transform = m_registry.get<TransformComponent>(entity);
		}

		auto view = m_registry.view<TransformComponent>();
		for (auto ent : view)
		{
			TransformComponent& transform = view.get<TransformComponent>(ent);
		}

		auto group = m_registry.group<TransformComponent>(entt::get<MeshComponent>);
		for (auto ent : group)
		{
			auto& [transform, mesh] = group.get<TransformComponent, MeshComponent>(ent);
		}

#endif


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

	void Scene::OnUpdate(Timestep ts)
	{

		
		// Render Scene
		Camera* mainCam = nullptr;
		glm::mat4* mainTransform = nullptr;
		{
			auto view = m_registry.view<TransformComponent, CameraComponent>();
			for (auto [entity, transform, camera] : view.each())
			{

				//auto& [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{

					mainCam = &camera.Cam;
					mainTransform = &transform.Transform;
					break;
				}

			}
		}

		if (mainCam)
		{

			Renderer2D::BeginScene(mainCam->GetProjection(), *mainTransform);
			{
				UG_PROFILE_SCOPE("Scene Renderer Draw");
				auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
				for (auto ent : group)
				{
					auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(ent);

					Renderer2D::DrawQuad(transform, sprite.Color);

			}
			Renderer2D::EndScene();

			


			}


		}


		


	}

}