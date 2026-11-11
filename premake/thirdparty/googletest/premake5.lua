-- Premake integration for GoogleTest / GoogleMock.
-- Only the "-all" translation units are compiled: gtest-all.cc and gmock-all.cc
-- #include the remaining sources, so listing the rest would duplicate symbols.
-- The *_main.cc entry points are deliberately left out so these libraries can be
-- linked into applications that already define main(); a test runner should call
-- ::testing::InitGoogleTest / ::testing::InitGoogleMock from its own main.

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
