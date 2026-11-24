-- Premake script for the Jolt static library.
-- CPU compute only; the DX12, Metal and Vulkan compute sources are left out.

project "Jolt"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"
	warnings "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"Jolt/**.h",
		"Jolt/**.cpp",
		"Jolt/**.inl",
		"Jolt/**.natvis"
	}

	removefiles
	{
		"Jolt/Jolt.cmake",
		"Jolt/Compute/DX12/**",
		"Jolt/Compute/MTL/**",
		"Jolt/Compute/VK/**"
	}

	includedirs
	{
		"."
	}

	defines(JoltDefines.Common)

	filter "system:windows"
		systemversion "latest"
		defines
		{
			"_CRT_SECURE_NO_WARNINGS"
		}
		buildoptions
		{
			"/utf-8"
		}

	filter "system:linux"
		pic "On"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		defines(JoltDefines.Debug)

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
		defines(JoltDefines.Release)

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
		defines(JoltDefines.Dist)

