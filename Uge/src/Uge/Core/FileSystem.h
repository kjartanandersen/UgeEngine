/**
 * @file FileSystem.h
 * @brief Minimal file helpers used by the importers and the project system.
 * @ingroup group_core
 */

#pragma once

#include <filesystem>

#include "Uge/Core/Buffer.h"

namespace Uge
{
	// TODO: Platforms
	/**
	 * @brief Static file utilities.
	 * @ingroup group_core
	 *
	 * @todo Currently Windows-only in practice; needs per-platform implementations.
	 */
	class FileSystem
	{


	public:
		/**
		 * @brief Reads an entire file into memory.
		 * @param filepath Path to the file to read.
		 * @return A buffer owning the file's bytes, or an empty buffer if the file could not
		 *         be opened. **The caller must Release() it**, or wrap it in a ScopedBuffer.
		 */
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);

		// Ensures an empty file exists at filepath, creating parent directories as needed.
		// Leaves the file untouched if it already exists. Returns true on success.
		/**
		 * @brief Ensures an empty file exists at @p filepath, creating parent directories.
		 * @param filepath Path of the file to create.
		 * @return `true` on success.
		 * @note Leaves the file untouched if it already exists.
		 */
		static bool CreateEmptyFile(const std::filesystem::path& filepath);

	};

}
