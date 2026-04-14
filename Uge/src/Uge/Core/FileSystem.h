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

	};

}
