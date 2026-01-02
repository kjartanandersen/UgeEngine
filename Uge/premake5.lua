project "Uge"
	location "./"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
	objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "ugpch.h"
	pchsource "src/ugpch.cpp"

	files 
	{
		"src/**.h",
		"src/**.cpp",
		"thirdparty/stb_image/**.h",
		"thirdparty/stb_image/**.cpp",
		"UgeClassDiagram.cd"
	}
	
	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"../%{prj.name}/thirdparty/spdlog/include",
		"../%{prj.name}/src",
		"../%{IncludeDir.GLFW}",
		"../%{IncludeDir.GLAD}",
		"../%{IncludeDir.IMGUI}",
		"../%{IncludeDir.GLM}",
		"../%{IncludeDir.STBI}"
	}

	links
	{
		"GLFW",
		"GLAD",
		"ImGui",
		"glm",
		"opengl32.lib"
	}
	
	buildoptions {"/utf-8"}

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"UG_PLATFORM_WINDOWS",
			"UG_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
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


	filter { "system:windows", "configurations:Release" }
		buildoptions "/MT"
