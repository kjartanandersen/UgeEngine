project "Sandbox"
	location "./"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"../%{IncludeDir.IMGUI}",
		"../%{IncludeDir.GLM}",
		"../%{IncludeDir.ENTT}"
	}

	links
	{
		"Uge",
		"ImGui",
		"glm"
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
		
	
