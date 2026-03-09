#include <ugpch.h>
#include "UUID.h"

#include <random>
#include <unordered_map>

static std::unordered_map<Uge::UUID, std::string> s_map;

static void AddToMap()
{
	s_map[Uge::UUID()] = "";
}

namespace Uge
{

	static std::random_device s_randomDevice;
	static std::mt19937_64 s_engine(s_randomDevice());
	static std::uniform_int_distribution<uint64_t> s_uniformDistribution;



	UUID::UUID()
		: m_UUID(s_uniformDistribution(s_engine))
	{
	}

	UUID::UUID(uint64_t uuid)
	{
	}

}