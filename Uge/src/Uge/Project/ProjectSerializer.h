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
	 * @brief Reads and writes a project's settings as YAML.
	 * @ingroup group_project
	 */
	class ProjectSerializer
	{

	public:
		/**
		 * @brief Creates a serializer for a project.
		 * @param project Project to read into or write from.
		 */
		ProjectSerializer(Ref<Project> project);

		/**
		 * @brief Writes the settings.
		 * @param filepath The `.ugproj` file; overwritten if it exists.
		 * @return `false` if the file couldn't be written.
		 */
		bool Serialize(const std::filesystem::path& filepath);
		/**
		 * @brief Reads the settings.
		 * @param filepath The `.ugproj` file.
		 * @return `false` if the file is missing or isn't valid YAML, has no `Project`
		 * section, lacks `Name`, `AssetDirectory` or `ScriptModulePath`, or has a value of the
		 * wrong type. The settings are then unchanged.
		 *
		 * A `StartScene` that isn't a scene handle is ignored with a warning.
		 */
		bool Deserialize(const std::filesystem::path& filepath);

	private:
		Ref<Project> m_project;

	};

}