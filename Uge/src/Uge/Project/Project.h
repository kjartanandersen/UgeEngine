#pragma once

#include <string>
#include <filesystem>

#include "Uge/Core/Core.h"
#include "Uge/Core/Log.h"

namespace Uge
{

	struct ProjectConfig
	{
		std::string Name = "Untitled";

		std::filesystem::path StartScene;

		std::filesystem::path AssetDirectory;
		std::filesystem::path ScriptModulePath;

	};

	class Project
	{

	public:

		static std::filesystem::path GetAssetDirectory()
		{

			UG_CORE_ASSERT(s_activeProject);
			return GetProjectDirectory() / s_activeProject->m_config.AssetDirectory;

		}

		static const std::filesystem::path& GetProjectDirectory() 
		{
			UG_CORE_ASSERT(s_activeProject);
			return s_activeProject->m_ProjectDirectory;

		}

		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{

			UG_CORE_ASSERT(s_activeProject);
			return GetAssetDirectory() / path;

		}

		static std::filesystem::path GetRelativePath(const std::filesystem::path& path)
		{

			return std::filesystem::relative(path, GetAssetDirectory());

		}

		static std::filesystem::path GetAssetAbsolutePath()
		{

			UG_CORE_ASSERT(s_activeProject);
			std::cout << "Current path is " << std::filesystem::current_path() << '\n';
			std::filesystem::path absPath(std::filesystem::absolute(GetAssetDirectory()));
			return std::filesystem::absolute(GetAssetDirectory());
			

		}

		ProjectConfig& GetConfig() { return m_config; }

		static Ref<Project> GetActive() { return s_activeProject; }

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);

	private:
		ProjectConfig m_config;
		std::filesystem::path m_ProjectDirectory;

		inline static Ref<Project> s_activeProject;

	};

}