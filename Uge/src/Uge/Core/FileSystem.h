#pragma once

#include <filesystem>

#include "Uge/Core/Buffer.h"

namespace Uge
{
	// TODO: Platforms
	class FileSystem
	{


	public:
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);

		// Ensures an empty file exists at filepath, creating parent directories as needed.
		// Leaves the file untouched if it already exists. Returns true on success.
		static bool CreateEmptyFile(const std::filesystem::path& filepath);

	};

}
