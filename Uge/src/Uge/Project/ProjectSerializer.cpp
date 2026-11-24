#include <ugpch.h>
#include "ProjectSerializer.h"


#include <fstream>

#include <yaml-cpp/yaml.h>

namespace Uge
{
	ProjectSerializer::ProjectSerializer(Ref<Project> project)
		: m_project(project)
	{
	}

	bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
	{
		auto& config = m_project->GetConfig();


		YAML::Emitter out;

		out << YAML::BeginMap;		// Root
		{
			out << YAML::Key << "Project"			<< YAML::Value;
			out << YAML::BeginMap;		// Project
			{
				out << YAML::Key << "Name"				<< YAML::Value << config.Name;
				out << YAML::Key << "StartScene"		<< YAML::Value << (uint64_t)config.StartScene;
				out << YAML::Key << "AssetDirectory"	<< YAML::Value << config.AssetDirectory.string();
				out << YAML::Key << "AssetRegistryPath"	<< YAML::Value << config.AssetRegistryPath.string();
				out << YAML::Key << "ScriptModulePath"	<< YAML::Value << config.ScriptModulePath.string();

			}
			out << YAML::EndMap;		// Project
		}
		out << YAML::EndMap;		// Root

		std::ofstream fout(filepath);
		fout << out.c_str();
		fout.close();

		// Covers a file that couldn't be opened, and a failed write or flush.
		if (fout.fail())
		{
			UG_CORE_ERROR("Failed to write .ugproj file {0}", filepath.string());
			return false;
		}

		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		// yaml-cpp throws a YAML::Exception for a missing or unreadable file, malformed
		// YAML, a subscript into a scalar or a value of the wrong type. Each returns false.
		try
		{
			YAML::Node data = YAML::LoadFile(filepath.string());

			YAML::Node projectNode = data["Project"];
			if (!projectNode)
			{
				UG_CORE_ERROR("Failed to load .ugproj file {0}: it has no Project section", filepath.string());
				return false;
			}

			for (const char* key : { "Name", "AssetDirectory", "ScriptModulePath" })
			{
				if (!projectNode[key])
				{
					UG_CORE_ERROR("Failed to load .ugproj file {0}: it has no {1}", filepath.string(), key);
					return false;
				}
			}

			// Read into a copy, so a file that fails halfway changes nothing.
			ProjectConfig config;
			config.Name = projectNode["Name"].as<std::string>();
			config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
			config.ScriptModulePath = projectNode["ScriptModulePath"].as<std::string>();
			if (projectNode["AssetRegistryPath"])
			{
				config.AssetRegistryPath = projectNode["AssetRegistryPath"].as<std::string>();
			}

			// StartScene is optional. Anything but a scene handle, such as a path, is
			// ignored, and the project opens on an empty scene.
			if (YAML::Node startScene = projectNode["StartScene"])
			{
				uint64_t handle = 0;
				if (YAML::convert<uint64_t>::decode(startScene, handle))
				{
					config.StartScene = handle;
				}
				else
				{
					UG_CORE_WARN("{0}: StartScene '{1}' is not a scene handle, so the project opens on an empty scene",
						filepath.string(), startScene.Scalar());
				}
			}

			m_project->GetConfig() = config;
			return true;
		}
		catch (const YAML::Exception& e)
		{
			UG_CORE_ERROR("Failed to load .ugproj file {0}\n	{1}", filepath.string(), e.what());
			return false;
		}
	}
}