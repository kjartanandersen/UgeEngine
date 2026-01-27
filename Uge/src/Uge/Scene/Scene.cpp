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

		// Update Scripts
		{

			m_registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				if (!nsc.Instance)
				{
					nsc.InstantiateFunction();
					nsc.Instance->m_entity = Entity{ entity, this };
					nsc.OnCreateFunction(nsc.Instance);
				}

				nsc.OnUpdateFunction(nsc.Instance, ts);

			});

		}


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

	void Uge::Scene::OnViewportResize(uint32_t width, uint32_t height)
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

}