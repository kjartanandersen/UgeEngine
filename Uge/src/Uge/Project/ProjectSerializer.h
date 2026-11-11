/**
 * @file ProjectSerializer.h
 * @brief Reads and writes `.ugproj` project files.
 * @ingroup group_project
 */

#pragma once


#include "Project.h"

namespace Uge
{

	/**
	 * @brief Serializes a Uge::ProjectConfig to YAML, and back.
	 * @ingroup group_project
	 *
	 * Handles only the project settings. The asset registry is a separate file, written by
	 * Uge::EditorAssetManager::SerializeAssetRegistry.
	 */
	class ProjectSerializer
	{

	public:
		/**
		 * @brief Binds the serializer to a project.
		 * @param project Project to read into or write from.
		 */
		ProjectSerializer(Ref<Project> project);
		
		/**
		 * @brief Writes the project configuration as YAML.
		 * @param filepath Destination `.ugproj` path; overwritten if it exists.
		 * @return `true` if the file was written; `false` if it couldn't be opened or written.
		 */
		bool Serialize(const std::filesystem::path& filepath);
		/**
		 * @brief Reads a project configuration from YAML.
		 * @param filepath Source `.ugproj` path.
		 * @return `true` on success. `false` if the file is missing, isn't valid YAML, has no
		 * `Project` section, lacks `Name`, `AssetDirectory` or `ScriptModulePath`, or holds a
		 * value of the wrong type; the project's settings are then left unchanged.
		 *
		 * A `StartScene` that isn't a scene handle is ignored with a warning, so the project
		 * opens on an empty scene.
		 */
		bool Deserialize(const std::filesystem::path& filepath);

	private:
		Ref<Project> m_project;

	};

}