/**
 * @file ScriptFieldTypeTests.cpp
 * @brief Tests for mapping C# field types to Uge::ScriptFieldType.
 */

#include <ugpch.h>

#include "Uge/Scripting/ScriptEngine.h"

#include <gtest/gtest.h>

#include <utility>

using Uge::ScriptFieldType;
using Uge::Utils::ScriptFieldTypeFromManagedName;

TEST(ScriptFieldTypeTest, MapsTheNamesMonoGivesCSharpTypes)
{
	// Mono uses .NET names: `int` is System.Int32 and `float` is System.Single.
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
	// The engine can't store a generic list.
	EXPECT_EQ(ScriptFieldTypeFromManagedName("System.Collections.Generic.List`1<System.Int32>"), ScriptFieldType::None);
}

TEST(ScriptFieldTypeTest, EverySerializedNameReadsBack)
{
	// Scene files store these names, so each parses back to its type.
	const ScriptFieldType types[] = {
		ScriptFieldType::Float, ScriptFieldType::Double,
		ScriptFieldType::Vector2, ScriptFieldType::Vector3, ScriptFieldType::Vector4,
		ScriptFieldType::Int, ScriptFieldType::UInt, ScriptFieldType::Long, ScriptFieldType::ULong,
		ScriptFieldType::Short, ScriptFieldType::UShort, ScriptFieldType::Bool,
		ScriptFieldType::Byte, ScriptFieldType::UByte, ScriptFieldType::Char,
		ScriptFieldType::String, ScriptFieldType::Entity,
	};

	for (ScriptFieldType type : types)
	{
		const char* name = Uge::Utils::ScriptFieldTypeToString(type);
		EXPECT_EQ(Uge::Utils::ScriptFieldTypeFromString(name), type) << name;
	}
}

TEST(ScriptFieldTypeTest, AnUnknownSerializedNameIsNone)
{
	// A scene file can name a type the engine doesn't know.
	EXPECT_EQ(Uge::Utils::ScriptFieldTypeFromString("Quaternion"), ScriptFieldType::None);
}