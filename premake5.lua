workspace "Uge"
	architecture "x64"

	configurations 
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include Directories relative to root folder (Solution Directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Uge/thirdparty/GLFW/include"
IncludeDir["GLAD"] = "Uge/thirdparty/GLAD/include"

include "Uge/thirdparty/GLFW"
include "Uge/thirdparty/GLAD"

project "Uge"
	location "Uge"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "ugpch.h"
	pchsource "Uge/src/ugpch.cpp"

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/thirdparty/spdlog/include",
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}"
	}

	links
	{
		"GLFW",
		"GLAD",
		"opengl32.lib",
		"dwmapi.lib"
	}
	
	buildoptions {"/utf-8"}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines 
		{
			"UG_PLATFORM_WINDOWS",
			"UG_BUILD_DLL",
			"UG_ENABLE_ASSERTS",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
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


	filter { "system:windows", "configurations:Release" }
		buildoptions "/MT"


project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files 
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Uge/thirdparty/spdlog/include",
		"Uge/src"
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
		
	