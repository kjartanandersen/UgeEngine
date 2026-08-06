/**
 * @file TestMain.cpp
 * @brief Entry point of the Uge test runner.
 */

#include <ugpch.h>

#include "Uge/Core/Log.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	// Engine code logs freely, and UG_CORE_* dereferences the logger unconditionally,
	// so the loggers have to exist before the first test runs.
	Uge::Log::Init();

	::testing::InitGoogleMock(&argc, argv);
	const int result = RUN_ALL_TESTS();

	Uge::Log::Shutdown();

	return result;
}
