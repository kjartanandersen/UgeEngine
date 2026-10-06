#include <ugpch.h>
#include "UUID.h"

#include <random>
#include <unordered_map>

namespace Uge
{

	static std::random_device s_randomDevice;
	static std::mt19937_64 s_engine(s_randomDevice());
	static std::uniform_int_distribution<uint64_t> s_uniformDistribution;

	UUID UUID::Generate()
	{
		// 0 is the null identifier, so it is never handed out. The odds are 2^-64 per call,
		// but the loop costs nothing.
		uint64_t value;
		do
		{
			value = s_uniformDistribution(s_engine);
		} while (value == 0);

		return UUID(value);
	}

	UUID::UUID(uint64_t uuid)
		: m_UUID(uuid)
	{
	}

}