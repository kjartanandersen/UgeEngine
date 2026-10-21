/**
 * @file UUIDTests.cpp
 * @brief Tests for Uge::UUID: the null identifier and generation.
 */

#include <ugpch.h>

#include "Uge/Asset/Asset.h"
#include "Uge/Core/UUID.h"

#include <gtest/gtest.h>

#include <unordered_set>

TEST(UUIDTest, DefaultConstructedIsTheNullIdentifier)
{
	// A default-constructed identifier is the null identifier, 0: "no entity", "no asset".
	EXPECT_EQ((uint64_t)Uge::UUID(), 0ull);
	EXPECT_EQ((uint64_t)Uge::AssetHandle(), 0ull);
}

TEST(UUIDTest, GenerateGivesDistinctNonNullIdentifiers)
{
	std::unordered_set<uint64_t> seen;
	for (int i = 0; i < 1000; i++)
	{
		const uint64_t id = Uge::UUID::Generate();
		EXPECT_NE(id, 0ull);
		EXPECT_TRUE(seen.insert(id).second);
	}
}