project "Uge"
	location "./"
	kind "SharedLib"
	language "C++"
	staticruntime "off"

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
		"../%{IncludeDir.IMGUI}",
		"../%{IncludeDir.GLM}"
	}

	links
	{
		"GLFW",
		"GLAD",
		"ImGui",
		"glm",
		"opengl32.lib",
		"dwmapi.lib"
	}
	
	buildoptions {"/utf-8"}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines 
		{
			"UG_PLATFORM_WINDOWS",
			"UG_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("IF NOT EXIST \"../bin/" .. outputdir .. "/Sandbox\" (mkdir \"../bin/" .. outputdir .. "/Sandbox\")"),
			("{COPYFILE} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
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


	filter { "system:windows", "configurations:Release" }
		buildoptions "/MT"
