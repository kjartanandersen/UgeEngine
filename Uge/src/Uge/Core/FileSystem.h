/**
 * @file FileSystem.h
 * @brief File helpers.
 * @ingroup group_core
 */

#pragma once

#include <filesystem>

#include "Uge/Core/Buffer.h"

namespace Uge
{
	/**
	 * @brief Static file helpers.
	 * @ingroup group_core
	 *
	 * @todo Windows only.
	 */
	class FileSystem
	{


	public:
		/**
		 * @brief Reads a whole file.
		 * @param filepath File to read.
		 * @return The bytes, or an empty buffer if the file can't be opened or is empty. The
		 * caller must Release() it.
		 */
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);

		/**
		 * @brief Creates an empty file, and its parent directories, unless it exists.
		 * @param filepath File to create.
		 * @return `true` if the file exists afterwards.
		 */
		static bool CreateEmptyFile(const std::filesystem::path& filepath);

	};

}
