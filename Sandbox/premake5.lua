project "Sandbox"
	location "./"
	kind "ConsoleApp"
	language "C++"
	staticruntime "off"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"../Uge/thirdparty/spdlog/include",
		"../Uge/src",
		"../Uge/thirdparty"
	}

	links
	{
		"Uge",
		"ImGui"
	}
	
	buildoptions {"/utf-8"}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines 
		{
			"UG_PLATFORM_WINDOWS"
		}


	filter "configurations:Debug"
		defines "UG_DEBUG"
		runtime "Debug"
		symbols "On"
		
	
	filter "configurations:Release"
		defines "UG_RELEASE"
		runtime "Release"
		optimize "On"
	
	filter "configurations:Dist"
		defines "UG_DIST"
		runtime "Release"
		optimize "On"
		
	