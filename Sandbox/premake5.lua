project "Sandbox"
	location "./"
	kind "ConsoleApp"
	language "C++"

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
		"../Uge/src"
	}

	links
	{
		"Uge"
	}
	
	buildoptions {"/utf-8"}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines 
		{
			"UG_PLATFORM_WINDOWS"
		}


	filter "configurations:Debug"
		defines "UG_DEBUG"
		buildoptions "/MDd"
		symbols "On"
		
	
	filter "configurations:Release"
		defines "UG_RELEASE"
		buildoptions "/MD"
		optimize "On"
	
	filter "configurations:Dist"
		defines "UG_DIST"
		buildoptions "/MD"
		optimize "On"
		
	