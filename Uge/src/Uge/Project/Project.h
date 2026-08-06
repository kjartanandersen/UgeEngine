/**
 * @file Project.h
 * @brief The active project: its configuration, paths and asset manager.
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
	 * @brief Settings stored in a `.ugproj` file.
	 * @ingroup group_project
	 *
	 * All paths are relative to the project directory — the folder containing the
	 * `.ugproj` — so a project can be moved or checked out anywhere.
	 */
	struct ProjectConfig
	{
		std::string Name = "Untitled"; ///< Display name of the project.

		AssetHandle StartScene; ///< Scene opened when the project loads.

		std::filesystem::path AssetDirectory; ///< Content root, relative to the project directory.
		std::filesystem::path AssetRegistryPath; ///< Asset registry file, relative to the asset directory.
		std::filesystem::path ScriptModulePath; ///< C# script assembly loaded by Uge::ScriptEngine.

	};

	/**
	 * @brief The loaded project: a content root, its asset manager and its settings.
	 * @ingroup group_project
	 *
	 * A global singleton, since the engine works with one project at a time. Most members
	 * are static and assert that a project is active, so the whole asset system depends on
	 * one having been loaded first.
	 *
	 * @code
	 * Ref<Project> project = Project::Load("SandboxProject/Sandbox.ugproj");
	 * auto scene = AssetManager::GetAsset<Scene>(project->GetConfig().StartScene);
	 * @endcode
	 *
	 * @warning Compose content paths with the `GetAsset*Path*` helpers rather than by hand,
	 * so relative and absolute forms stay consistent.
	 */
	class Project
	{

	public:

		/**
		 * @brief The project's content root.
		 * @return Absolute path to the asset directory.
		 */
		static std::filesystem::path GetAssetDirectory()
		{

			UG_CORE_ASSERT(s_activeProject);
			return GetProjectDirectory() / s_activeProject->m_config.AssetDirectory;

		}

		/**
		 * @brief The directory containing the `.ugproj` file.
		 * @return Const reference to the project directory.
		 */
		static const std::filesystem::path& GetProjectDirectory() 
		{
			UG_CORE_ASSERT(s_activeProject);
			return s_activeProject->m_ProjectDirectory;

		}

		/**
		 * @brief Where the asset registry is serialized.
		 * @return Path to the registry file, inside the asset directory.
		 */
		static std::filesystem::path GetAssetRegistryPath()
		{

			UG_CORE_ASSERT(s_activeProject);
			return GetAssetDirectory() / s_activeProject->m_config.AssetRegistryPath;

		}

		/**
		 * @brief Resolves an asset-relative path against the content root.
		 * @param path Path relative to the asset directory.
		 * @return The corresponding filesystem path.
		 */
		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{

			UG_CORE_ASSERT(s_activeProject);
			return GetAssetDirectory() / path;

		}

		/**
		 * @brief Makes a filesystem path relative to the content root.
		 * @param path Absolute or working-directory-relative path.
		 * @return The path relative to the asset directory, as stored in the registry.
		 */
		static std::filesystem::path GetRelativePath(const std::string& path)
		{

			return std::filesystem::relative(path, GetAssetDirectory());

		}

		/**
		 * @brief GetRelativePath() as a string.
		 * @param path Path to convert.
		 * @return The relative path in string form.
		 */
		static std::string GetRelativePathString(const std::string& path)
		{
			std::string sPath = (GetRelativePath(path).string());
			return sPath;

		}

		/**
		 * @brief The content root as an absolute path.
		 * @return Absolute path to the asset directory.
		 */
		static std::filesystem::path GetAssetAbsolutePath()
		{

			UG_CORE_ASSERT(s_activeProject);
			std::cout << "Current path is " << std::filesystem::current_path() << '\n';
			return std::filesystem::absolute(GetAssetDirectory());
			

		}

		/**
		 * @brief Resolves an asset-relative path against the content root.
		 * @param path Path relative to the asset directory.
		 * @return The combined path.
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
		 * @return The combined path in string form.
		 */
		static std::string GetAssetAbsolutePathString(const std::string& path)
		{

			UG_CORE_ASSERT(s_activeProject);
			std::filesystem::path assetPath = GetAssetDirectory() / path;
			return assetPath.string();

		}

		/**
		 * @brief The project's settings.
		 * @return Mutable reference to the configuration.
		 */
		ProjectConfig& GetConfig() { return m_config; }

		/**
		 * @brief The currently loaded project.
		 * @return The active project, or null if none has been loaded.
		 */
		static Ref<Project> GetActive() { return s_activeProject; }
		/**
		 * @brief The project's asset manager.
		 * @return The manager, as the common base interface.
		 */
		Ref<AssetManagerBase> GetAssetManager() { return m_assetManager; }
		/**
		 * @brief The asset manager as a runtime manager.
		 * @return The manager downcast to Uge::RuntimeAssetManager.
		 * @warning Unchecked cast; only valid in a packaged build.
		 */
		Ref<RuntimeAssetManager> GetRuntimeAssetManager() { return std::static_pointer_cast<RuntimeAssetManager>(m_assetManager); }
		/**
		 * @brief The asset manager as an editor manager.
		 * @return The manager downcast to Uge::EditorAssetManager.
		 * @warning Unchecked cast; only valid when running in the editor.
		 */
		Ref<EditorAssetManager> GetEditorAssetManager() { return std::static_pointer_cast<EditorAssetManager>(m_assetManager); }

		/**
		 * @brief Creates an empty project and makes it active.
		 * @return The new project.
		 */
		static Ref<Project> New();
		/**
		 * @brief Loads a `.ugproj` file and makes it the active project.
		 * @param path Path to the project file.
		 * @return The loaded project, or null if it could not be read.
		 *
		 * Also creates the asset manager and deserializes the asset registry.
		 */
		static Ref<Project> Load(const std::filesystem::path& path);
		/**
		 * @brief Writes the active project to a `.ugproj` file.
		 * @param path Destination path.
		 * @return `true` on success.
		 */
		static bool SaveActive(const std::filesystem::path& path);

		/**
		 * @brief Releases the active project, returning to the no-project state.
		 *
		 * Afterwards GetActive() is null and the static path helpers assert again, exactly as
		 * before the first Load(). Nothing else clears the singleton, so this is the only way
		 * back to the state the editor is in before a project is opened.
		 *
		 * @warning Anything still holding assets from the project — an open scene, the content
		 * browser — is left pointing at a manager that is no longer reachable through the
		 * facade. Drop those first.
		 */
		static void Unload();

	private:
		ProjectConfig m_config;
		std::filesystem::path m_ProjectDirectory;

		Ref<AssetManagerBase> m_assetManager;

		inline static Ref<Project> s_activeProject;

	};

}