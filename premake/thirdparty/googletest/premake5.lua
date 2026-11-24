-- Premake script for GoogleTest and GoogleMock.
-- Only gtest-all.cc and gmock-all.cc are compiled; they include the other sources.
-- The *_main.cc files are left out, as the test runner has its own main().

project "googletest"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"
	warnings "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"googletest/include/**.h",
		"googletest/src/gtest-internal-inl.h",
		"googletest/src/gtest-all.cc"
	}

	includedirs
	{
		"googletest",
		"googletest/include"
	}

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

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"


project "googlemock"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"
	warnings "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"googlemock/include/**.h",
		"googlemock/src/gmock-all.cc"
	}

	includedirs
	{
		"googlemock",
		"googlemock/include",
		"googletest",
		"googletest/include"
	}

	links
	{
		"googletest"
	}

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

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
