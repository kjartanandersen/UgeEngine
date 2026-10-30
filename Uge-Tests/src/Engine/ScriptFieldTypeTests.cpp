/**
 * @file ScriptFieldTypeTests.cpp
 * @brief Tests for how C# field types map onto Uge::ScriptFieldType.
 */

#include <ugpch.h>

#include "Uge/Scripting/ScriptEngine.h"

#include <gtest/gtest.h>

#include <utility>

using Uge::ScriptFieldType;
using Uge::Utils::ScriptFieldTypeFromManagedName;

TEST(ScriptFieldTypeTest, MapsTheNamesMonoGivesCSharpTypes)
{
	// Mono names a field's type the .NET way, not by its C# keyword: an `int` is
	// System.Int32 and a `float` is System.Single.
	const std::pair<const char*, ScriptFieldType> names[] = {
		{ "System.Single",  ScriptFieldType::Float },
		{ "System.Double",  ScriptFieldType::Double },
		{ "System.Boolean", ScriptFieldType::Bool },
		{ "System.Char",    ScriptFieldType::Char },
		{ "System.SByte",   ScriptFieldType::Byte },
		{ "System.Byte",    ScriptFieldType::UByte },
		{ "System.Int16",   ScriptFieldType::Short },
		{ "System.UInt16",  ScriptFieldType::UShort },
		{ "System.Int32",   ScriptFieldType::Int },
		{ "System.UInt32",  ScriptFieldType::UInt },
		{ "System.Int64",   ScriptFieldType::Long },
		{ "System.UInt64",  ScriptFieldType::ULong },
		{ "System.String",  ScriptFieldType::String },
		{ "Uge.Vector2",    ScriptFieldType::Vector2 },
		{ "Uge.Vector3",    ScriptFieldType::Vector3 },
		{ "Uge.Vector4",    ScriptFieldType::Vector4 },
		{ "Uge.Entity",     ScriptFieldType::Entity },
	};

	for (const auto& [name, type] : names)
	{
		EXPECT_EQ(ScriptFieldTypeFromManagedName(name), type) << name;
	}
}

TEST(ScriptFieldTypeTest, AnUnsupportedTypeIsNone)
{
	// A generic list is a valid field type in C#, but not one the engine can store.
	EXPECT_EQ(ScriptFieldTypeFromManagedName("System.Collections.Generic.List`1<System.Int32>"), ScriptFieldType::None);
}