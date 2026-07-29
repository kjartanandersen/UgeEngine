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
		 * @return `true` on success.
		 */
		bool Serialize(const std::filesystem::path& filepath);
		/**
		 * @brief Reads a project configuration from YAML.
		 * @param filepath Source `.ugproj` path.
		 * @return `true` on success; `false` if the file is missing or malformed.
		 */
		bool Deserialize(const std::filesystem::path& filepath);

	private:
		Ref<Project> m_project;

	};

}