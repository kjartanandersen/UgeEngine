#pragma once

#include <string>
#include <filesystem>

#include "Uge/Core/Core.h"
#include "Uge/Core/Log.h"
#include "Uge/Asset/RuntimeAssetManager.h"
#include "Uge/Asset/EditorAssetManager.h"

namespace Uge
{

	struct ProjectConfig
	{
		std::string Name = "Untitled";

		AssetHandle StartScene;

		std::filesystem::path AssetDirectory;
		std::filesystem::path AssetRegistryPath;
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

		static std::filesystem::path GetAssetRegistryPath()
		{

			UG_CORE_ASSERT(s_activeProject);
			return GetAssetDirectory() / s_activeProject->m_config.AssetRegistryPath;

		}

		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{

			UG_CORE_ASSERT(s_activeProject);
			return GetAssetDirectory() / path;

		}

		static std::filesystem::path GetRelativePath(const std::string& path)
		{

			return std::filesystem::relative(path, GetAssetDirectory());

		}

		static std::string GetRelativePathString(const std::string& path)
		{
			std::string sPath = (std::filesystem::relative(path, GetAssetDirectory()).string());
			return sPath;

		}

		static std::filesystem::path GetAssetAbsolutePath()
		{

			UG_CORE_ASSERT(s_activeProject);
			std::cout << "Current path is " << std::filesystem::current_path() << '\n';
			return std::filesystem::absolute(GetAssetDirectory());
			

		}

		static std::filesystem::path GetAssetAbsolutePath(const std::string& path)
		{

			UG_CORE_ASSERT(s_activeProject);
			std::filesystem::path assetPath = GetAssetDirectory() / path;
			return assetPath;

		}

		static std::string GetAssetAbsolutePathString(const std::string& path)
		{

			UG_CORE_ASSERT(s_activeProject);
			std::filesystem::path assetPath = GetAssetDirectory() / path;
			return assetPath.string();

		}

		ProjectConfig& GetConfig() { return m_config; }

		static Ref<Project> GetActive() { return s_activeProject; }
		Ref<AssetManagerBase> GetAssetManager() { return m_assetManager; }
		Ref<RuntimeAssetManager> GetRuntimeAssetManager() { return std::static_pointer_cast<RuntimeAssetManager>(m_assetManager); }
		Ref<EditorAssetManager> GetEditorAssetManager() { return std::static_pointer_cast<EditorAssetManager>(m_assetManager); }

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);

	private:
		ProjectConfig m_config;
		std::filesystem::path m_ProjectDirectory;

		Ref<AssetManagerBase> m_assetManager;

		inline static Ref<Project> s_activeProject;

	};

}