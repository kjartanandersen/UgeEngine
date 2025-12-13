project "GLAD"
	kind "StaticLib"
	language "C"
	staticruntime "off"
	warnings "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	IncludeDirGLAD = {}
	IncludeDirGLAD["GLAD"] = "include"
	
	includedirs
	{
		
		"%{IncludeDirGLAD.GLAD}"
	}

	files
	{
		"include/glad/glad.h",
		"include/KHR/khrplatform.h",
		"src/glad.c"
	}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		staticruntime "on"
        runtime "Debug" -- This sets the runtime library to /MTd for Debug builds

	filter { "system:windows", "configurations:Debug-AS" }	
		runtime "Debug"
		symbols "on"
		sanitize { "Address" }
		flags { "NoRuntimeChecks", "NoIncrementalLink" }

	filter "configurations:Release"
		runtime "Release"
		optimize "speed"
		staticruntime "on"
        runtime "Release" -- This sets the runtime library to /MT for Release builds

    filter "configurations:Dist"
		runtime "Release"
		optimize "speed"
        symbols "off"
