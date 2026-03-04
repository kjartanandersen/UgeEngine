#include <ugpch.h>
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components.h"

#include <fstream>

#include <yaml-cpp/yaml.h>


namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	

}

namespace Uge
{


	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{

		out << YAML::BeginMap;	// Entity
		out << YAML::Key << "Entity" << YAML::Value << "12313141561654"; // TODO: Entity ID goes here

		// Get tag component
		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Cam;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthoSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthoNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthoFarClip();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent

			auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
			

			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (entity.HasComponent<MeshComponent>())
		{
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap; // MeshComponent

			auto& meshComponent = entity.GetComponent<MeshComponent>();
			out << YAML::Key << "Path" << YAML::Value << meshComponent.FilePath;

			out << YAML::EndMap; // MeshComponent
		}



		out << YAML::EndMap;	// Entity


	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_scene(scene) {}

	void SceneSerializer::Serialize(const std::string& filepath)
	{

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		for (auto entityID : m_scene->m_registry.view<entt::entity>())
		{
			Entity entity = { entityID, m_scene.get() };
			if (!entity)
			{
				return;
			}

			SerializeEntity(out, entity);
		
		
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();






	}
	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		
		// Not implemented
		UG_CORE_ASSERT(false);

	}
	bool SceneSerializer::DeSerialize(const std::string& filepath)
	{

		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
		{
			return false;
		}

		std::string sceneName = data["Scene"].as<std::string>();
		UG_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (entities)
		{

			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto ytgc = entity["TagComponent"];		// Tag Component
				if (ytgc)
				{
					name = ytgc["Tag"].as<std::string>();
				}

				UG_CORE_TRACE("Deserialized entity with ID {0} and name {1}", uuid, name);

				Entity deserializedEntity = m_scene->CreateEntity(name);

				auto ytc = entity["TransformComponent"];	// Tranform Component
				if (ytc)
				{

					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = ytc["Translation"].as<glm::vec3>();
					tc.Rotation = ytc["Rotation"].as<glm::vec3>();
					tc.Scale = ytc["Scale"].as<glm::vec3>();

				}

				auto ycc = entity["CameraComponent"];	// Camera Component
				if (ycc)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();

					auto& cameraProps = ycc["Camera"];
					cc.Cam.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

					cc.Cam.SetPerspVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.Cam.SetPerspNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Cam.SetPerspFarClip(cameraProps["PerspectiveFar"].as<float>());

					cc.Cam.SetOrthoSize(cameraProps["OrthographicSize"].as<float>());
					cc.Cam.SetOrthoNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Cam.SetOrthoFarClip(cameraProps["OrthographicFar"].as<float>());

					cc.Primary = ycc["Primary"].as<bool>();
					cc.FixedAspectRatio = ycc["FixedAspectRatio"].as<bool>();
				}

				auto ysrc = entity["SpriteRendererComponent"];		// Sprite Renderer Component
				if (ysrc)
				{
					auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
					src.Color = ysrc["Color"].as<glm::vec4>();

				}

				auto ymc = entity["MeshComponent"];		// Mesh Component
				if (ymc)
				{
					std::string path = ymc["Path"] ? ymc["Path"].as<std::string>() : std::string();
					deserializedEntity.AddComponent<MeshComponent>(path);
				}

			}

		}

		return true;
	}
	bool SceneSerializer::DeSerializeRuntime(const std::string& filepath)
	{


		// Not implemented
		UG_CORE_ASSERT(false);
		return false;
	}
}
