/**
 * @file TempProject.h
 * @brief A temporary project on disk.
 */

#pragma once

#include "Uge/Project/Project.h"

#include <filesystem>
#include <string>

namespace UgeTests
{

	/**
	 * @brief Creates an empty project in a temporary directory and loads it, for tests that
	 * use Uge::AssetManager. Unloads and deletes it on destruction.
	 */
	class TempProject
	{
	public:
		/**
		 * @brief Creates the directories, saves the `.ugproj` and loads it.
		 * @param name Project name and `.ugproj` file stem.
		 */
		explicit TempProject(const std::string& name = "TestProject");

		/** @brief Unloads the project and removes its directory. */
		~TempProject();

		TempProject(const TempProject&) = delete;
		TempProject& operator=(const TempProject&) = delete;

		/** @brief The project directory. @return The path. */
		const std::filesystem::path& Root() const { return m_root; }
		/** @brief The `.ugproj` file. @return The path. */
		const std::filesystem::path& ProjectFile() const { return m_projectFile; }
		/** @brief The asset directory. @return The path. */
		std::filesystem::path AssetDirectory() const { return m_root / "Assets"; }

		/**
		 * @brief Writes a file in the asset directory, creating its parent directories.
		 * @param relativePath Path relative to the asset directory.
		 * @param contents The text.
		 * @return The file's absolute path.
		 */
		std::filesystem::path WriteAssetFile(const std::filesystem::path& relativePath,
			const std::string& contents) const;

	private:
		std::filesystem::path m_root;
		std::filesystem::path m_projectFile;
	};

}
