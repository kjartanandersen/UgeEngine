/**
 * @file ScriptEngineTests.cpp
 * @brief Tests for Uge::ScriptEngine without Mono.
 */

#include <ugpch.h>

#include "Uge/Scripting/ScriptEngine.h"

#include <gtest/gtest.h>

#include <filesystem>

namespace
{
	/** @brief The script engine as it starts from the repo root: no Mono, field storage only. */
	class ScriptEngineTest : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			if (std::filesystem::exists("mono/lib"))
			{
				GTEST_SKIP() << "Mono's runtime is in the working directory; run the tests from the repo root.";
			}
			Uge::ScriptEngine::Init();
		}

		void TearDown() override { Uge::ScriptEngine::Shutdown(); }
	};
}

TEST_F(ScriptEngineTest, LoadingAndUnloadingAProjectWorkWithoutMono)
{
	Uge::ScriptEngine::UnloadProject();
	Uge::ScriptEngine::LoadProject();
	Uge::ScriptEngine::UnloadProject();
}

TEST_F(ScriptEngineTest, UnloadingAProjectForgetsItsFieldValues)
{
	Uge::ScriptFieldMap* fields = Uge::ScriptEngine::GetScriptFieldMap(Uge::UUID(42));
	ASSERT_NE(fields, nullptr);

	Uge::ScriptFieldInstance& speed = (*fields)["Speed"];
	speed.Field.Name = "Speed";
	speed.Field.Type = Uge::ScriptFieldType::Float;
	speed.SetValue(2.5f);

	Uge::ScriptEngine::UnloadProject();

	fields = Uge::ScriptEngine::GetScriptFieldMap(Uge::UUID(42));
	ASSERT_NE(fields, nullptr);
	EXPECT_TRUE(fields->empty());
}