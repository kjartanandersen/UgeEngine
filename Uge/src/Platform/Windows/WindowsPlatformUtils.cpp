#include <ugpch.h>
#include "Uge/Utils/PlatformUtils.h"
#include "Uge/Core/Application.h"

#include <commdlg.h>
#include <GLFW/glfw3.h>

#include <cstring>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


namespace Uge
{
	namespace
	{
		void ExtractDefaultExtension(const char* filter, char* output, size_t outputSize)
		{
			if (!filter || !output || outputSize == 0)
			{
				return;
			}

			const char* pattern = filter + std::strlen(filter) + 1;
			if (*pattern == '\0')
			{
				return;
			}

			while (*pattern == '*' || *pattern == '.')
			{
				pattern++;
			}

			size_t index = 0;
			while (*pattern != '\0' && *pattern != ';' && index + 1 < outputSize)
			{
				output[index++] = *pattern++;
			}

			output[index] = '\0';
		}
	}

	std::string FileDialogs::OpenFile(const char* filter)
	{

		OPENFILENAMEA ofn;			// Common dialog box structure
		CHAR szFile[260] = { 0 };	// If using TCHAR Macros
		CHAR currentDir[256] = { 0 };

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
		{
			ofn.lpstrInitialDir = currentDir;
		}

		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}

		return std::string();


	}
	std::string FileDialogs::SaveFile(const char* filter)
	{
		OPENFILENAMEA ofn;			// Common dialog box structure
		CHAR szFile[260] = { 0 };	// If using TCHAR Macros
		CHAR currentDir[256] = { 0 };
		CHAR defaultExt[32] = { 0 };

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);

		if (GetCurrentDirectoryA(256, currentDir))
		{
			ofn.lpstrInitialDir = currentDir;
		}

		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
		ExtractDefaultExtension(filter, defaultExt, sizeof(defaultExt));
		ofn.lpstrDefExt = defaultExt[0] != '\0' ? defaultExt : nullptr;

		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}

		return std::string();

	}
	 

}
