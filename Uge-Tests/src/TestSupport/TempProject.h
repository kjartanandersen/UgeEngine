/**
 * @file TempProject.h
 * @brief A throwaway Uge project on disk, for anything that needs an active project.
 */

#pragma once

#include "Uge/Project/Project.h"

#include <filesystem>
#include <string>

namespace UgeTests
{

	/**
	 * @brief Creates a real project in a temporary directory and makes it active.
	 *
	 * Most of the asset system reaches the active project through Uge::AssetManager, which
	 * asserts when there is none, so a test touching assets, the asset registry or scene
	 * serialization needs one to exist. Pointing tests at the editor's SandboxProject would
	 * make them depend on — and write to — real content, so this builds a fresh empty one
	 * per test instead.
	 *
	 * The project is unloaded and the directory deleted on destruction, which is what keeps
	 * a project from leaking into tests that expect the no-project state.
	 */
	class TempProject
	{
	public:
		/**
		 * @brief Creates the directory tree, writes the `.ugproj` and loads it.
		 * @param name Project name, also used for the `.ugproj` file stem.
		 */
		explicit TempProject(const std::string& name = "TestProject");

		/** @brief Unloads the project and removes its directory. */
		~TempProject();

		TempProject(const TempProject&) = delete;
		TempProject& operator=(const TempProject&) = delete;

		/** @brief The project directory. @return Path to the temporary root. */
		const std::filesystem::path& Root() const { return m_root; }
		/** @brief The `.ugproj` file. @return Path to the project file. */
		const std::filesystem::path& ProjectFile() const { return m_projectFile; }
		/** @brief The content root. @return Path to the project's asset directory. */
		std::filesystem::path AssetDirectory() const { return m_root / "Assets"; }

		/**
		 * @brief Writes a file into the asset directory, creating parent directories.
		 * @param relativePath Path relative to the asset directory.
		 * @param contents Text to write.
		 * @return The absolute path of the file written.
		 */
		std::filesystem::path WriteAssetFile(const std::filesystem::path& relativePath,
			const std::string& contents) const;

	private:
		std::filesystem::path m_root;
		std::filesystem::path m_projectFile;
	};

}
