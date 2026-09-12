#include <ugpch.h>
#include "RenderStats.h"

namespace Uge
{

	namespace
	{
		RenderStats s_stats;
	}

	RenderStats& RenderStats::Get()
	{
		return s_stats;
	}

	void RenderStats::Reset()
	{
		s_stats = RenderStats{};
	}

}
