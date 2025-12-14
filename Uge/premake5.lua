project "Uge"
	location "./"
	kind "SharedLib"
	language "C++"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "ugpch.h"
	pchsource "src/ugpch.cpp"

	files 
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"../%{prj.name}/thirdparty/spdlog/include",
		"../%{prj.name}/src",
		"../%{IncludeDir.GLFW}",
		"../%{IncludeDir.GLAD}",
		"../%{IncludeDir.IMGUI}"
	}

	links
	{
		"GLFW",
		"GLAD",
		"ImGui",
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