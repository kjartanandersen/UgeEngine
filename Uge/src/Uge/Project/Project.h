/**
 * @file Project.h
 * @brief The active project: its settings, paths and asset manager.
 * @ingroup group_project
 */

#pragma once

#include <string>
#include <filesystem>

#include "Uge/Core/Core.h"
#include "Uge/Core/Log.h"
#include "Uge/Asset/RuntimeAssetManager.h"
#include "Uge/Asset/EditorAssetManager.h"

namespace Uge
{

	/**
	 * @brief The settings in a `.ugproj` file. Its paths are relative, so a project can move.
	 * @ingroup group_project
	 */
	struct ProjectConfig
	{
		std::string Name = "Untitled"; ///< Project name.

		AssetHandle StartScene; ///< Scene opened on load.

		std::filesystem::path AssetDirectory; ///< Relative to the project directory.
		std::filesystem::path AssetRegistryPath; ///< Relative to the asset directory.
		std::filesystem::path ScriptModulePath; ///< The C# script assembly.

	};

	/**
	 * @brief The loaded project.
	 * @ingroup group_project
	 *
	 * One project is active at a time. The static functions assert when none is.
	 */
	class Project
	{

	public:

		/**
		 * @brief The asset directory.
		 * @return Its path.
		 */
		static std::filesystem::path GetAssetDirectory()
		{

			UG_CORE_ASSERT(s_activeProject);
			return GetProjectDirectory() / s_activeProject->m_config.AssetDirectory;

		}

		/**
		 * @brief The folder holding the `.ugproj` file.
		 * @return The folder.
		 */
		static const std::filesystem::path& GetProjectDirectory() 
		{
			UG_CORE_ASSERT(s_activeProject);
			return s_activeProject->m_ProjectDirectory;

		}

		/**
		 * @brief The asset registry file.
		 * @return Its path.
		 */
		static std::filesystem::path GetAssetRegistryPath()
		{

			UG_CORE_ASSERT(s_activeProject);
			return GetAssetDirectory() / s_activeProject->m_config.AssetRegistryPath;

		}

		/**
		 * @brief Where an asset path is on disk.
		 * @param path Path relative to the asset directory.
		 * @return The path on disk.
		 */
		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{

			UG_CORE_ASSERT(s_activeProject);
			return GetAssetDirectory() / path;

		}

		/**
		 * @brief A path relative to the asset directory.
		 * @param path Path on disk.
		 * @return The relative path.
		 */
		static std::filesystem::path GetRelativePath(const std::string& path)
		{

			return std::filesystem::relative(path, GetAssetDirectory());

		}

		/**
		 * @brief GetRelativePath() as a string.
		 * @param path Path on disk.
		 * @return The relative path.
		 */
		static std::string GetRelativePathString(const std::string& path)
		{
			std::string sPath = (GetRelativePath(path).string());
			return sPath;

		}

		/**
		 * @brief The asset directory as an absolute path.
		 * @return The path.
		 */
		static std::filesystem::path GetAssetAbsolutePath()
		{

			UG_CORE_ASSERT(s_activeProject);
			std::cout << "Current path is " << std::filesystem::current_path() << '\n';
			return std::filesystem::absolute(GetAssetDirectory());
			

		}

		/**
		 * @brief Where an asset path is on disk.
		 * @param path Path relative to the asset directory.
		 * @return The path on disk.
		 */
		static std::filesystem::path GetAssetAbsolutePath(const std::string& path)
		{

			UG_CORE_ASSERT(s_activeProject);
			std::filesystem::path assetPath = GetAssetDirectory() / path;
			return assetPath;

		}

		/**
		 * @brief GetAssetAbsolutePath() as a string.
		 * @param path Path relative to the asset directory.
		 * @return The path on disk.
		 */
		static std::string GetAssetAbsolutePathString(const std::string& path)
		{

			UG_CORE_ASSERT(s_activeProject);
			std::filesystem::path assetPath = GetAssetDirectory() / path;
			return assetPath.string();

		}

		/**
		 * @brief The settings.
		 * @return The settings.
		 */
		ProjectConfig& GetConfig() { return m_config; }

		/**
		 * @brief The active project.
		 * @return The project, or null if none is loaded.
		 */
		static Ref<Project> GetActive() { return s_activeProject; }
		/**
		 * @brief The asset manager.
		 * @return The manager.
		 */
		Ref<AssetManagerBase> GetAssetManager() { return m_assetManager; }
		/**
		 * @brief The asset manager as a Uge::RuntimeAssetManager. Unchecked; packaged builds only.
		 * @return The manager.
		 */
		Ref<RuntimeAssetManager> GetRuntimeAssetManager() { return std::static_pointer_cast<RuntimeAssetManager>(m_assetManager); }
		/**
		 * @brief The asset manager as a Uge::EditorAssetManager. Unchecked; editor only.
		 * @return The manager.
		 */
		Ref<EditorAssetManager> GetEditorAssetManager() { return std::static_pointer_cast<EditorAssetManager>(m_assetManager); }

		/**
		 * @brief Creates an empty project and makes it active.
		 * @return The project.
		 *
		 * It has no directory or asset manager until it is saved with SaveActive() and
		 * opened with Load().
		 */
		static Ref<Project> New();
		/**
		 * @brief Opens a `.ugproj` file and makes it the active project.
		 * @param path The project file.
		 * @return The project, or null if it can't be read; the previous project then stays
		 * active.
		 *
		 * Creates the asset manager and reads the asset registry.
		 */
		static Ref<Project> Load(const std::filesystem::path& path);
		/**
		 * @brief Saves the active project.
		 * @param path The `.ugproj` file.
		 * @return `true` on success. On failure nothing is changed.
		 */
		static bool SaveActive(const std::filesystem::path& path);

		/**
		 * @brief Closes the active project; GetActive() is null afterwards.
		 *
		 * Release anything that holds its assets first.
		 */
		static void Unload();

	private:
		ProjectConfig m_config;
		std::filesystem::path m_ProjectDirectory;

		Ref<AssetManagerBase> m_assetManager;

		inline static Ref<Project> s_activeProject;

	};

}