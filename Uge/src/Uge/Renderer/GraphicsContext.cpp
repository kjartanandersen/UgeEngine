#include <ugpch.h>
#include "GraphicsContext.h"

namespace Uge
{

	GraphicsDeviceInfo GraphicsContext::s_deviceInfo;

	const GraphicsDeviceInfo& GraphicsContext::GetDeviceInfo()
	{
		return s_deviceInfo;
	}

	void GraphicsContext::SetDeviceInfo(const GraphicsDeviceInfo& info)
	{
		s_deviceInfo = info;
	}

}
