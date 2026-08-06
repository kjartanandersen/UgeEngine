-- GoogleTest runner covering both the engine and the editor.
--
-- The editor is a ConsoleApp, so it cannot be linked into another executable. Its
-- sources are compiled into this project instead, which is why the include and define
-- set below mirrors UgeEditor/premake5.lua. UgeEditorApp.cpp is the one file left out:
-- it pulls in EntryPoint.h, whose main() would clash with the test runner's.

project "Uge-Tests"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	-- Log::Init writes logs/Uge.log relative to the working directory.
	debugdir "%{wks.location}"

	files
	{
		"src/**.h",
		"src/**.cpp",

		"../UgeEditor/src/**.h",
		"../UgeEditor/src/**.cpp"
	}

	removefiles
	{
		"../UgeEditor/src/UgeEditorApp.cpp"
	}

	includedirs
	{
		"src",
		"%{wks.location}/UgeEditor/src",
		"%{wks.location}/Uge/thirdparty/spdlog/include",
		"%{wks.location}/Uge/src",
		"%{wks.location}/%{IncludeDir.IMGUI}",
		"%{wks.location}/%{IncludeDir.GLM}",
		"%{wks.location}/%{IncludeDir.ENTT}",
		"%{wks.location}/%{IncludeDir.FILEWATCH}",
		"%{wks.location}/%{IncludeDir.IMGUIZMO}",
		"%{wks.location}/%{IncludeDir.GOOGLETEST}",
		"%{wks.location}/%{IncludeDir.GOOGLEMOCK}"
	}

	links
	{
		"Uge",
		"googletest",
		"googlemock"
	}

	buildoptions {"/utf-8"}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"UG_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines { "UG_DEBUG", "UG_PROFILE=1" }
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines { "UG_RELEASE", "UG_PROFILE=1" }
		runtime "Release"
		optimize "on"
		symbols "on"

	filter "configurations:Dist"
		defines "UG_DIST"
		runtime "Release"
		optimize "on"
