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
	// A handle declared without an initializer must mean "none", not a random id that points
	// at nothing and gets saved that way (SCN-16).
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