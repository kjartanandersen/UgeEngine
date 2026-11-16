/**
 * @file TestMain.cpp
 * @brief The test runner's entry point.
 */

#include <ugpch.h>

#include "Uge/Core/Log.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	// The log macros need the loggers to exist.
	Uge::Log::Init();

	::testing::InitGoogleMock(&argc, argv);
	const int result = RUN_ALL_TESTS();

	Uge::Log::Shutdown();

	return result;
}
