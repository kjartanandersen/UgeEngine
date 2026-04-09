project "Uge-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/Uge/thirdparty/spdlog/include",
		"%{wks.location}/Uge/src",
		"%{wks.location}/%{IncludeDir.IMGUI}",
		"%{wks.location}/%{IncludeDir.GLM}",
		"%{wks.location}/%{IncludeDir.ENTT}",
		"%{wks.location}/%{IncludeDir.FILEWATCH}",
		"%{wks.location}/%{IncludeDir.IMGUIZMO}"
	}

	links
	{
		"Uge"
	}
	
	buildoptions {"/utf-8"}
	

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"UG_PLATFORM_WINDOWS"
		}


	filter "configurations:Debug"
		defines "UG_DEBUG"
		runtime "Debug"
		symbols "on"
		
	
	filter "configurations:Release"
		defines "UG_RELEASE"
		runtime "Release"
		optimize "on"
	
	filter "configurations:Dist"
		defines "UG_DIST"
		runtime "Release"
		optimize "on"
		
	
