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
	 * @brief Native open and save dialogs. They block the frame loop until closed.
	 * @ingroup group_platform
	 */
	class FileDialogs
	{

	public:
		/**
		 * @brief Shows the open dialog.
		 * @param filter Win32 filter: description and pattern pairs, each ending in `\0`, such
		 *        as `"Uge Scene (*.uge)\0*.uge\0"`.
		 * @return The chosen path, or an empty string if cancelled.
		 */
		static std::string OpenFile(const char* filter);
		/**
		 * @brief Shows the save dialog.
		 * @param filter Win32 filter, as for OpenFile().
		 * @return The chosen path, or an empty string if cancelled.
		 */
		static std::string SaveFile(const char* filter);
	};

}