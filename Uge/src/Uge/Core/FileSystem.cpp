#include <ugpch.h>
#include "FileSystem.h"

namespace Uge
{

	Buffer FileSystem::ReadFileBinary(const std::filesystem::path& filepath)
	{

		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			// Failed to open the file
			return {};
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint32_t size = (uint32_t)end - (uint32_t)stream.tellg();
		if (size == 0)
		{
			// File is empty
			return {};
		}

		Buffer buffer(size);
		stream.read(buffer.As<char>(), size);
		stream.close();

		return buffer;


	}

	bool FileSystem::CreateEmptyFile(const std::filesystem::path& filepath)
	{
		if (std::filesystem::exists(filepath))
		{
			return true;
		}

		std::error_code ec;
		if (filepath.has_parent_path())
		{
			std::filesystem::create_directories(filepath.parent_path(), ec);
		}

		std::ofstream file(filepath);
		if (!file)
		{
			UG_CORE_ERROR("FileSystem::CreateEmptyFile - Failed to create file {0}", filepath.string());
			return false;
		}

		return true;
	}

}