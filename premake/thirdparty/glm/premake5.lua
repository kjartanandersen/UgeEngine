project "glm"
	kind "StaticLib"
	language "C++"
	staticruntime "on"
	warnings "off"
	cppdialect "C++20"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"glm/**.hpp",
		"glm/**.h",
		"glm/**.inl",
		"glm/**.cpp",
		"*.txt",
		"*.md",
		"util/glm.natvis"
	}

	includedirs
	{
		"glm"
	}

	filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
