#pragma once

#include "Uge/Core/Core.h"
#include "Uge/Core/UUID.h"
#include "Uge/Renderer/Model.h"
#include "SceneCamera.h"
#include "Uge/Renderer/Texture.h"

#include <string>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Uge
{

	struct IDComponent
	{
		UUID ID;
		IDComponent() = default;
		IDComponent(const IDComponent&) = default;

	};

	struct TagComponent
	{

		std::string Tag; 

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}


	};

	struct MeshComponent
	{
		std::string FilePath;
		Ref<Model> ModelAsset;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		explicit MeshComponent(const std::string& filePath)
		{
			SetModel(filePath);
		}

		void SetModel(const std::string& filePath)
		{
			FilePath = filePath;
			if (FilePath.empty())
			{
				UG_CORE_WARN("MeshComponent::SetModel called with no filePath");
				ModelAsset = nullptr;
				return;
			}

			ModelAsset = CreateRef<Model>(FilePath);
		}

		bool HasModel() const
		{
			return (bool)ModelAsset;
		}

	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };



		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			glm::mat4 translation = glm::translate(glm::mat4(1.0f), Translation);

			glm::mat4 rotation = glm::mat4_cast(glm::quat(Rotation));

			glm::mat4 scale = glm::scale(glm::mat4(1.0f), Scale);

			return translation * rotation * scale;
		}

	};

	
	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}

	};

	
	struct CameraComponent
	{

		SceneCamera Cam;
		bool Primary = true; // TODO: Move to scene
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;


	};

	// Forward Decleration
	class ScriptableEntity;

	struct NativeScriptComponent
	{

		ScriptableEntity* Instance = nullptr;



		ScriptableEntity*(*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);


		template<typename T>
		void Bind()
		{

			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };

		}


	};


}




