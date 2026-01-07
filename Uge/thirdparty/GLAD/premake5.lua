project "GLAD"
	kind "StaticLib"
	language "C"
	staticruntime "on"
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
        runtime "Debug" -- This sets the runtime library to /MTd for Debug builds

	filter { "system:windows", "configurations:Debug-AS" }	
		runtime "Debug"
		symbols "on"
		sanitize { "Address" }
		runtimechecks "off"
		incrementallink "off"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
        runtime "Release" -- This sets the runtime library to /MT for Release builds

    filter "configurations:Dist"
		runtime "Release"
		optimize "on"
        symbols "off"
