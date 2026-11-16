-- The GoogleTest runner for the engine and the editor.
--
-- The editor is an executable, so its sources are compiled in here, with the includes and
-- defines of UgeEditor/premake5.lua. UgeEditorApp.cpp is left out, as it defines main().

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
		"%{wks.location}/%{IncludeDir.JOLT}",
		"%{wks.location}/%{IncludeDir.GOOGLETEST}",
		"%{wks.location}/%{IncludeDir.GOOGLEMOCK}"
	}

	links
	{
		"Uge",
		"Jolt",
		"googletest",
		"googlemock"
	}

	buildoptions {"/utf-8"}

	defines(JoltDefines.Common) 

	filter "system:windows"
		systemversion "latest"


	filter "configurations:Debug"
		defines { "UG_DEBUG", "UG_PROFILE=1" }
		defines(JoltDefines.Debug)
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines { "UG_RELEASE", "UG_PROFILE=1" }
		defines(JoltDefines.Release)
		runtime "Release"
		optimize "on"
		symbols "on"

	filter "configurations:Dist"
		defines "UG_DIST"
		defines(JoltDefines.Dist)
		runtime "Release"
		optimize "on"
