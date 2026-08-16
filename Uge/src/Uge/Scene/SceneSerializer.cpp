#include <ugpch.h>
#include "SceneSerializer.h"

#include "Uge/Scene/Entity.h"
#include "Uge/Scene/Components.h"
#include "Uge/Scripting/ScriptEngine.h"

#include "Uge/Project/Project.h"

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

	template<>
	struct convert<Uge::UUID>
	{
		static Node encode(const Uge::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, Uge::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};

	

}

namespace Uge
{

#define WRITE_SCRIPT_FIELD(FieldType, Type)           \
			case ScriptFieldType::FieldType:          \
				out << scriptField.GetValue<Type>();  \
				break

#define READ_SCRIPT_FIELD(FieldType, Type)             \
	case ScriptFieldType::FieldType:                   \
	{                                                  \
		Type data = scriptField["Data"].as<Type>();    \
		fieldInstance.SetValue(data);                  \
		break;                                         \
	}



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
		UG_CORE_ASSERT(entity.HasComponent<IDComponent>());

		out << YAML::BeginMap;	// Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

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

		if (entity.HasComponent<ScriptComponent>())
		{
			out << YAML::Key << "ScriptComponent";

			out << YAML::BeginMap; // ScriptComponent

			auto& scriptComponent = entity.GetComponent<ScriptComponent>();

			out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;

			// Fields
			Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
			const auto& fields = entityClass->GetFields();
			if (fields.size() > 0)
			{
				out << YAML::Key << "ScriptFields" << YAML::Value;

				auto& entityFields = ScriptEngine::GetScriptFieldMap(entity.GetUUID());
				out << YAML::BeginSeq;
				for (const auto& [name, field] : fields)
				{
					if (entityFields.find(name) == entityFields.end())
					{
						continue;
					}


					out << YAML::BeginMap; // ScriptField
					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);

					out << YAML::Key << "Data" << YAML::Value;
					ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(Float,	float		);
						WRITE_SCRIPT_FIELD(Double,	double		);
						WRITE_SCRIPT_FIELD(Bool,	bool		);
						WRITE_SCRIPT_FIELD(Char,	char		);
						WRITE_SCRIPT_FIELD(Byte,	int8_t		);
						WRITE_SCRIPT_FIELD(Short,	int16_t		);
						WRITE_SCRIPT_FIELD(Int,		int32_t		);
						WRITE_SCRIPT_FIELD(Long,	int64_t		);
						WRITE_SCRIPT_FIELD(UByte,	uint8_t		);
						WRITE_SCRIPT_FIELD(UShort,	uint16_t	);
						WRITE_SCRIPT_FIELD(UInt,	uint32_t	);
						WRITE_SCRIPT_FIELD(ULong,	uint64_t	);
						WRITE_SCRIPT_FIELD(Vector2, glm::vec2	);
						WRITE_SCRIPT_FIELD(Vector3, glm::vec3	);
						WRITE_SCRIPT_FIELD(Vector4, glm::vec4	);
						WRITE_SCRIPT_FIELD(Entity,	UUID		);
					}
					out << YAML::EndMap; // ScriptFields

				}
				out << YAML::EndSeq;
			}
			

			out << YAML::EndMap; // ScriptComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent

			auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
			// TODO: Add texture support
			
			out << YAML::Key << "TextureHandle" << YAML::Value << spriteRendererComponent.Texture;

			
			out << YAML::Key << "TilingFactor" << YAML::Value << spriteRendererComponent.TilingFactor;


			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (entity.HasComponent<MeshComponent>())
		{
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap; // MeshComponent

			auto& meshComponent = entity.GetComponent<MeshComponent>();
			out << YAML::Key << "MeshHandle" << YAML::Value << meshComponent.Mesh;

			out << YAML::EndMap; // MeshComponent
		}

		if (entity.HasComponent<SkyLightComponent>())
		{
			out << YAML::Key << "SkyLightComponent";
			out << YAML::BeginMap; // SkyLightComponent

			auto& skyLightComponent = entity.GetComponent<SkyLightComponent>();
			out << YAML::Key << "EnvironmentHandle" << YAML::Value << skyLightComponent.Environment;
			out << YAML::Key << "Intensity" << YAML::Value << skyLightComponent.Intensity;

			out << YAML::EndMap; // SkyLightComponent
		}

		if (entity.HasComponent<DirectionalLightComponent>())
		{
			out << YAML::Key << "DirectionalLightComponent";
			out << YAML::BeginMap; // DirectionalLightComponent

			auto& directionalLightComponent = entity.GetComponent<DirectionalLightComponent>();
			out << YAML::Key << "Color" << YAML::Value << directionalLightComponent.Color;
			out << YAML::Key << "Intensity" << YAML::Value << directionalLightComponent.Intensity;

			out << YAML::EndMap; // DirectionalLightComponent
		}

		if (entity.HasComponent<RigidbodyComponent>())
		{
			out << YAML::Key << "RigidbodyComponent";
			out << YAML::BeginMap; // RigidbodyComponent

			auto& rb = entity.GetComponent<RigidbodyComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << (int)rb.Type;
			out << YAML::Key << "Layer" << YAML::Value << (int)rb.Layer;
			out << YAML::Key << "Mass" << YAML::Value << rb.Mass;
			out << YAML::Key << "LinearDamping" << YAML::Value << rb.LinearDamping;
			out << YAML::Key << "AngularDamping" << YAML::Value << rb.AngularDamping;
			out << YAML::Key << "GravityFactor" << YAML::Value << rb.GravityFactor;
			out << YAML::Key << "FixedRotation" << YAML::Value << rb.FixedRotation;

			out << YAML::EndMap; // RigidbodyComponent
		}

		if (entity.HasComponent<BoxColliderComponent>())
		{

			out << YAML::Key << "BoxColliderComponent";
			out << YAML::BeginMap; // BoxColliderComponent

			auto& bc = entity.GetComponent<BoxColliderComponent>();
			out << YAML::Key << "HalfExtents" << YAML::Value << bc.HalfExtents;
			out << YAML::Key << "IsTrigger" << YAML::Value << bc.IsTrigger;

			out << YAML::Key << "Material" << YAML::Value;
			out << YAML::BeginMap; // Material

				out << YAML::Key << "Density" << YAML::Value << bc.Material.Density;
				out << YAML::Key << "Friction" << YAML::Value << bc.Material.Friction;
				out << YAML::Key << "Restitution" << YAML::Value << bc.Material.Restitution;


			out << YAML::EndMap; // Material

			out << YAML::Key << "Offset" << YAML::Value << bc.Offset;


			out << YAML::EndMap; // RigidbodyComponent

		}

		if (entity.HasComponent<SphereColliderComponent>())
		{

			out << YAML::Key << "SphereColliderComponent";
			out << YAML::BeginMap; // SphereColliderComponent

			auto& scc = entity.GetComponent<SphereColliderComponent>();
			out << YAML::Key << "IsTrigger" << YAML::Value << scc.IsTrigger;
			
			out << YAML::Key << "Material" << YAML::Value;
			out << YAML::BeginMap; // Material

				out << YAML::Key << "Density" << YAML::Value << scc.Material.Density;
				out << YAML::Key << "Friction" << YAML::Value << scc.Material.Friction;
				out << YAML::Key << "Restitution" << YAML::Value << scc.Material.Restitution;


			out << YAML::EndMap; // Material
			
			out << YAML::Key << "Offset" << YAML::Value << scc.Offset;
			out << YAML::Key << "Radius" << YAML::Value << scc.Radius;


			out << YAML::EndMap; // SphereColliderComponent

		}

		if (entity.HasComponent<CapsuleColliderComponent>())
		{

			out << YAML::Key << "SphereColliderComponent";
			out << YAML::BeginMap; // CapsuleColliderComponent

			auto& ccc = entity.GetComponent<CapsuleColliderComponent>();
			out << YAML::Key << "HalfHeight"	<< YAML::Value << (int)ccc.HalfHeight;
			out << YAML::Key << "IsTrigger"		<< YAML::Value << ccc.IsTrigger;

			out << YAML::Key << "Material" << YAML::Value;
			out << YAML::BeginMap; // Material

				out << YAML::Key << "Density"		<< YAML::Value << ccc.Material.Density;
				out << YAML::Key << "Friction"		<< YAML::Value << ccc.Material.Friction;
				out << YAML::Key << "Restitution"	<< YAML::Value << ccc.Material.Restitution;


			out << YAML::EndMap; // Material

			out << YAML::Key << "Offset"		<< YAML::Value << ccc.Offset;
			out << YAML::Key << "Radius"		<< YAML::Value << ccc.Radius;
				

			out << YAML::EndMap; // CapsuleColliderComponent

		}

		if (entity.HasComponent<MeshColliderComponent>())
		{

			out << YAML::Key << "SphereColliderComponent";
			out << YAML::BeginMap; // MeshColliderComponent

			auto& ccc = entity.GetComponent<MeshColliderComponent>();
			out << YAML::Key << "Convex"		<< YAML::Value << (int)ccc.Convex;
			out << YAML::Key << "IsTrigger"		<< YAML::Value << ccc.IsTrigger;

			out << YAML::Key << "Material" << YAML::Value;
			out << YAML::BeginMap; // Material

				out << YAML::Key << "Density"		<< YAML::Value << ccc.Material.Density;
				out << YAML::Key << "Friction"		<< YAML::Value << ccc.Material.Friction;
				out << YAML::Key << "Restitution"	<< YAML::Value << ccc.Material.Restitution;


			out << YAML::EndMap; // Material

			out << YAML::Key << "Mesh"		<< YAML::Value << ccc.Mesh;


			out << YAML::EndMap; // MeshColliderComponent

		}

		if (entity.HasComponent<TextComponent>())
		{
			out << YAML::Key << "TextComponent";
			out << YAML::BeginMap; // TextComponent

			auto& textComponent = entity.GetComponent<TextComponent>();
			out << YAML::Key << "TextString" << YAML::Value << textComponent.TextString;
			// TODO: Font
			out << YAML::Key << "Kerning" << YAML::Value << textComponent.Kerning;
			out << YAML::Key << "LineSpacing" << YAML::Value << textComponent.LineSpacing;
			out << YAML::Key << "Color" << YAML::Value << textComponent.Color;

			out << YAML::EndMap; // TextComponent
		}

		out << YAML::EndMap;	// Entity


	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_scene(scene) {}

	void SceneSerializer::Serialize(const std::filesystem::path& filepath)
	{

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << Project::GetRelativePath(filepath.string()).string();
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
	void SceneSerializer::SerializeRuntime(const std::filesystem::path& filepath)
	{
		
		// Not implemented
		UG_CORE_ASSERT(false);

	}
	bool SceneSerializer::DeSerialize(const std::filesystem::path& filepath)
	{
		/*
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
		{
			return false;
		}
		*/

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		// YAML::Exception rather than ParserException alone: a scene file that has been
		// moved or deleted raises YAML::BadFile, which is not a ParserException and would
		// otherwise be thrown all the way out through AssetManager::GetAsset.
		catch (const YAML::Exception& e)
		{
			UG_CORE_ERROR("Failed to load .uge file {0}\n	{1}", filepath.string(), e.what());
			return false;
		}
		if (!data["Scene"])
		{
			return false;
		}


		std::string sceneName = data["Scene"].as<std::string>();
		UG_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		m_scene->SetName(sceneName);

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

				Entity deserializedEntity = m_scene->CreateEntityWithUUID(uuid, name);

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
					UG_CORE_TRACE("Deserializing Camera Component");
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();

					auto& cameraProps = ycc["Camera"];
					cc.Cam.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

					cc.Cam.SetPerspVerticalFOV(	cameraProps["PerspectiveFOV"	].as<float>());
					cc.Cam.SetPerspNearClip(	cameraProps["PerspectiveNear"	].as<float>());
					cc.Cam.SetPerspFarClip(		cameraProps["PerspectiveFar"	].as<float>());

					cc.Cam.SetOrthoSize(		cameraProps["OrthographicSize"	].as<float>());
					cc.Cam.SetOrthoNearClip(	cameraProps["OrthographicNear"	].as<float>());
					cc.Cam.SetOrthoFarClip(		cameraProps["OrthographicFar"	].as<float>());

					cc.Primary = ycc["Primary"].as<bool>();
					cc.FixedAspectRatio = ycc["FixedAspectRatio"].as<bool>();
				}

				auto ysc = entity["ScriptComponent"];
				if (ysc)
				{
					UG_CORE_TRACE("Deserializing Script Component");

					auto& sc = deserializedEntity.AddComponent<ScriptComponent>();

					sc.ClassName = ysc["ClassName"].as<std::string>();

					auto scriptFields = ysc["ScriptFields"];
					if (scriptFields)
					{
						Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);
						if (entityClass)
						{
							const auto& fields = entityClass->GetFields();
							auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity.GetUUID());

							for (auto scriptField : scriptFields)
							{
								std::string name = scriptField["Name"].as<std::string>();
								std::string typeString = scriptField["Type"].as<std::string>();
								ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

								ScriptFieldInstance& fieldInstance = entityFields[name];

								// TODO: turn this assert into log warning
								// UG_CORE_ASSERT(fields.find(name) != fields.end());

								if (fields.find(name) == fields.end())
									continue;

								fieldInstance.Field = fields.at(name);

								switch (type)
								{
									READ_SCRIPT_FIELD(Float, float);
									READ_SCRIPT_FIELD(Double, double);
									READ_SCRIPT_FIELD(Bool, bool);
									READ_SCRIPT_FIELD(Char, char);
									READ_SCRIPT_FIELD(Byte, int8_t);
									READ_SCRIPT_FIELD(Short, int16_t);
									READ_SCRIPT_FIELD(Int, int32_t);
									READ_SCRIPT_FIELD(Long, int64_t);
									READ_SCRIPT_FIELD(UByte, uint8_t);
									READ_SCRIPT_FIELD(UShort, uint16_t);
									READ_SCRIPT_FIELD(UInt, uint32_t);
									READ_SCRIPT_FIELD(ULong, uint64_t);
									READ_SCRIPT_FIELD(Vector2, glm::vec2);
									READ_SCRIPT_FIELD(Vector3, glm::vec3);
									READ_SCRIPT_FIELD(Vector4, glm::vec4);
									READ_SCRIPT_FIELD(Entity, UUID);
								}
							}
						}
						
					}

				}

				auto ysrc = entity["SpriteRendererComponent"];		// Sprite Renderer Component
				if (ysrc)
				{
					UG_CORE_TRACE("Deserializing Sprite Renderer Component");

					auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
					src.Color = ysrc["Color"].as<glm::vec4>();

					if (ysrc["TextureHandle"])
					{
						src.Texture = ysrc["TextureHandle"].as<AssetHandle>();
					}
					if (ysrc["TilingFactor"])
					{
						src.TilingFactor = ysrc["TilingFactor"].as<float>();

					}

				}

				auto ymc = entity["MeshComponent"];		// Mesh Component
				if (ymc)
				{
					UG_CORE_TRACE("Deserializing Mesh Component");

					AssetHandle meshAsset = ymc["MeshHandle"] ? ymc["MeshHandle"].as<AssetHandle>() : AssetHandle();
					// std::filesystem::path path = Project::GetAssetFileSystemPath(meshPath);
					deserializedEntity.AddComponent<MeshComponent>(meshAsset);
				}

				auto yslc = entity["SkyLightComponent"];	// Sky Light Component
				if (yslc)
				{
					UG_CORE_TRACE("Deserializing Sky Light Component");

					auto& src = deserializedEntity.AddComponent<SkyLightComponent>();

					if (yslc["EnvironmentHandle"])
					{
						src.Environment = yslc["EnvironmentHandle"].as<AssetHandle>();
					}
					if (yslc["Intensity"])
					{
						src.Intensity = yslc["Intensity"].as<float>();
					}
				}

				auto ydlc = entity["DirectionalLightComponent"];	// Directional Light Component
				if (ydlc)
				{
					UG_CORE_TRACE("Deserializing Directional Light Component");

					auto& src = deserializedEntity.AddComponent<DirectionalLightComponent>();

					if (ydlc["Color"])
					{
						src.Color = ydlc["Color"].as<glm::vec3>();
					}
					if (ydlc["Intensity"])
					{
						src.Intensity = ydlc["Intensity"].as<float>();
					}
				}


				auto yrb = entity["RigidbodyComponent"];		// Rigid Body Component
				if (yrb)
				{
					UG_CORE_TRACE("Deserializing Rigidbody Component");

					auto& rb = deserializedEntity.AddComponent<RigidbodyComponent>();
					if (yrb["BodyType"])			rb.Type = (BodyType)yrb["BodyType"].as<int>();
					if (yrb["Layer"])				rb.Layer = (PhysicsLayer)yrb["Layer"].as<int>();
					if (yrb["Mass"])				rb.Mass = yrb["Mass"].as<float>();
					if (yrb["LinearDamping"])		rb.LinearDamping = yrb["LinearDamping"].as<float>();
					if (yrb["AngularDamping"])		rb.AngularDamping = yrb["AngularDamping"].as<float>();
					if (yrb["GravityFactor"])		rb.GravityFactor = yrb["GravityFactor"].as<float>();
					if (yrb["FixedRotation"])		rb.FixedRotation = yrb["FixedRotation"].as<bool>();
					
				}

				auto ybcc = entity["BoxColliderComponent"];		// Box Collider Component
				if (ybcc)
				{
					auto& bc = deserializedEntity.AddComponent<BoxColliderComponent>();
					if (ybcc["HalfExtents"])					bc.HalfExtents = ybcc["HalfExtents"].as<glm::vec3>();
					if (ybcc["IsTrigger"])						bc.IsTrigger = ybcc["IsTrigger"].as<bool>();

					if (ybcc["Material"])
					{
						PhysicsMaterial material;
						auto& ybccm = ybcc["Material"];
						if (ybccm["Density"])					material.Density = ybccm["Density"].as<float>();
						if (ybccm["Friction"])					material.Friction = ybccm["Friction"].as<float>();
						if (ybccm["Restitution"])				material.Restitution = ybccm["Restitution"].as<float>();

						bc.Material = material;
					}


				}

				auto yscc = entity["SphereColliderComponent"];		// Sphere Collider Component
				if (yscc)
				{
					auto& scc = deserializedEntity.AddComponent<SphereColliderComponent>();
					if (yscc["IsTrigger"])						scc.IsTrigger = yscc["IsTrigger"].as<bool>();
					if (yscc["Material"])
					{
						PhysicsMaterial material;
						auto& ysccm = ybcc["Material"];
						if (ysccm["Density"])					material.Density = ysccm["Density"].as<float>();
						if (ysccm["Friction"])					material.Friction = ysccm["Friction"].as<float>();
						if (ysccm["Restitution"])				material.Restitution = ysccm["Restitution"].as<float>();

						scc.Material = material;
					}
					if (yscc["Offset"])							scc.Offset = yscc["Offset"].as<glm::vec3>();
					if (yscc["Radius"])							scc.Radius = yscc["Radius"].as<float>();


				}

				auto yccc = entity["CapsuleColliderComponent"];		// Capsule Collider Component
				if (yccc)
				{

					auto& ccc = deserializedEntity.AddComponent<CapsuleColliderComponent>();
					if (yccc["HalfHeight"])						ccc.HalfHeight = yccc["HalfHeight"].as<float>();
					if (yccc["IsTrigger"])							ccc.IsTrigger = yccc["IsTrigger"].as<bool>();
					if (yccc["Material"])
					{
						PhysicsMaterial material;
						auto& ysccm = yccc["Material"];
						if (ysccm["Density"])					material.Density = ysccm["Density"].as<float>();
						if (ysccm["Friction"])					material.Friction = ysccm["Friction"].as<float>();
						if (ysccm["Restitution"])				material.Restitution = ysccm["Restitution"].as<float>();

						ccc.Material = material;
					}
					if (yccc["Offset"])							ccc.Offset = yccc["Offset"].as<glm::vec3>();
					if (yccc["Radius"])							ccc.Radius = yccc["Radius"].as<float>();

				}

				auto ymcc = entity["MeshColliderComponent"];		// Mesh Collider Component
				if (ymcc)
				{

					auto& mcc = deserializedEntity.AddComponent<MeshColliderComponent>();
					if (ymcc["Convex"])							mcc.Convex = ymcc["Convex"].as<bool>();
					if (ymcc["IsTrigger"])						mcc.IsTrigger = ymcc["IsTrigger"].as<bool>();
					if (ymcc["Material"])
					{
						PhysicsMaterial material;
						auto& ysccm = ymcc["Material"];
						if (ysccm["Density"])					material.Density = ysccm["Density"].as<float>();
						if (ysccm["Friction"])					material.Friction = ysccm["Friction"].as<float>();
						if (ysccm["Restitution"])				material.Restitution = ysccm["Restitution"].as<float>();

						mcc.Material = material;
					}
					if (ymcc["Mesh"])							mcc.Mesh = ymcc["Mesh"].as<AssetHandle>();

				}
				
				auto ytec = entity["TextComponent"];		// Text Component
				if (ytec)
				{
					UG_CORE_TRACE("Deserializing Text Component");

					auto& src = deserializedEntity.AddComponent<TextComponent>();
					src.TextString = ytec["TextString"].as<std::string>();

					src.Kerning = ytec["Kerning"].as<float>();
					src.LineSpacing = ytec["LineSpacing"].as<float>();
					
					src.Color = ytec["Color"].as<glm::vec4>();
					// src.Font = ; // TODO

				}

			}

		}

		return true;
	}
	bool SceneSerializer::DeSerializeRuntime(const std::filesystem::path& filepath)
	{


		// Not implemented
		UG_CORE_ASSERT(false);
		return false;
	}
}
