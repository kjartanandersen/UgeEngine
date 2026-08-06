project "Uge"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "ugpch.h"
	pchsource "src/ugpch.cpp"

	files 
	{
		"src/**.h",
		"src/**.cpp",
		"thirdparty/stb_image/**.h",
		"thirdparty/stb_image/**.cpp",

		"thirdparty/ImGuizmo/ImGuizmo.h",
		"thirdparty/ImGuizmo/ImGuizmo.cpp"
		
	}

	

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

	includedirs
	{
		"../%{prj.name}/thirdparty/spdlog/include",
		"../%{prj.name}/src",
		"../%{IncludeDir.GLFW}",
		"../%{IncludeDir.GLAD}",
		"../%{IncludeDir.IMGUI}",
		"../%{IncludeDir.GLM}",
		"../%{IncludeDir.MSDFGEN}",
		"../%{IncludeDir.MSDF_ATLAS_GEN}",
		"../%{IncludeDir.STBI}",
		"../%{IncludeDir.ENTT}",
		"../%{IncludeDir.MONO}",
		"../%{IncludeDir.FILEWATCH}",
		"../%{IncludeDir.ASSIMP}",
		"../%{IncludeDir.RAPIDJSON}",
		"../%{IncludeDir.YAMLCPP}",
		"../%{IncludeDir.JOLT}",
		"../%{IncludeDir.IMGUIZMO}",
		"../%{IncludeDir.VULKANSDK}",
		"../%{IncludeDir.GOOGLETEST}",
		"../%{IncludeDir.GOOGLEMOCK}"

	}

	links
	{
		"GLFW",
		"GLAD",
		"ImGui",
		"glm",
		"msdf-atlas-gen",
		"assimp",
		"yaml-cpp",
		"googletest",
		"googlemock",
		"opengl32.lib",

		"%{Library.MONO}"
	}
	
	
	buildoptions {"/utf-8"}
	
	defines
	{
		"YAML_CPP_STATIC_DEFINE"
	}
	
	filter "files:thirdparty/ImGuizmo/**.cpp"
		enablepch "off"
	

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"UG_PLATFORM_WINDOWS",
			"UG_BUILD_DLL"
		}

		links
		{
			"%{Library.WINSOCK}",
			"%{Library.WINMM}",
			"%{Library.WINVER}",
			"%{Library.WINCRYPT}",
			-- Stack walking and minidump writing for Uge::CrashHandler.
			"dbghelp.lib"
		}


	filter "configurations:Debug"
		defines
		{
			"UG_DEBUG",
			"UG_PROFILE=1"
		}
		runtime "Debug"
		symbols "on"
		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}
		
	
	filter "configurations:Release"
		defines
		{
			"UG_RELEASE",
			"UG_PROFILE=1"
		}
		runtime "Release"
		optimize "on"
		-- Keep PDBs in Release so Uge::CrashHandler can symbolise its stack traces.
		symbols "on"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

	
	filter "configurations:Dist"
		defines "UG_DIST"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}


		
