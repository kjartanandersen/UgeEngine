/**
 * @file PlatformUtils.h
 * @brief Native file dialogs.
 * @ingroup group_platform
 */

#pragma once

#include <string>

namespace Uge
{

	/**
	 * @brief Blocking native open/save dialogs.
	 * @ingroup group_platform
	 *
	 * Implemented per platform; the Windows version in
	 * `Platform/Windows/WindowsPlatformUtils.cpp` uses the Win32 common dialogs and
	 * parents them to the application window.
	 *
	 * @code
	 * std::string path = FileDialogs::OpenFile("Uge Scene (*.uge)\0*.uge\0");
	 * if (!path.empty())
	 *     OpenScene(path);
	 * @endcode
	 *
	 * @warning These block the calling thread — and therefore the frame loop — until the
	 * user dismisses the dialog.
	 */
	class FileDialogs
	{

	public:
		/**
		 * @brief Shows the native "open file" dialog.
		 * @param filter Win32 filter string: description and pattern pairs, each terminated by
		 *        a `\0`, e.g. `"Uge Scene (*.uge)\0*.uge\0"`.
		 * @return The absolute path chosen, or an empty string if the user cancelled.
		 */
		static std::string OpenFile(const char* filter);
		/**
		 * @brief Shows the native "save file" dialog.
		 * @param filter Win32 filter string, in the same form as for OpenFile().
		 * @return The absolute path chosen, or an empty string if the user cancelled.
		 */
		static std::string SaveFile(const char* filter);
	};

}