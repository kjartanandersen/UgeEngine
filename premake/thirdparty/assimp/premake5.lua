local function copy_if_missing(destination, source)
	if os.isfile(destination) then
		return
	end

	local ok, err = os.copyfile(source, destination)
	if not ok then
		error("assimp premake: failed to copy '" .. source .. "' to '" .. destination .. "' (" .. tostring(err) .. ")")
	end
end

local function ensure_config_header()
	local source_path = "include/assimp/config.h.in"
	local destination_path = "include/assimp/config.h"

	local source_file, source_err = io.open(source_path, "r")
	if not source_file then
		error("assimp premake: failed to read '" .. source_path .. "' (" .. tostring(source_err) .. ")")
	end

	local source_text = source_file:read("*a")
	source_file:close()

	-- Resolve CMake template directives for premake-generated builds.
	local generated_text = source_text:gsub("#cmakedefine%s+ASSIMP_DOUBLE_PRECISION%s+1", "/* undef ASSIMP_DOUBLE_PRECISION */")

	local current_file = io.open(destination_path, "r")
	if current_file then
		local current_text = current_file:read("*a")
		current_file:close()
		if current_text == generated_text then
			return
		end
	end

	local output_file, output_err = io.open(destination_path, "w")
	if not output_file then
		error("assimp premake: failed to write '" .. destination_path .. "' (" .. tostring(output_err) .. ")")
	end

	output_file:write(generated_text)
	output_file:close()
end

local function write_revision_if_missing(path)
	if os.isfile(path) then
		return
	end

	local file, err = io.open(path, "w")
	if not file then
		error("assimp premake: failed to create '" .. path .. "' (" .. tostring(err) .. ")")
	end

	file:write([[
	#ifndef ASSIMP_REVISION_H_INC
	#define ASSIMP_REVISION_H_INC

	#define GitVersion 0
	#define GitBranch "unknown"

	#define VER_MAJOR 6
	#define VER_MINOR 0
	#define VER_PATCH 4
	#define VER_BUILD 0

	#define STR_HELP(x) #x
	#define STR(x) STR_HELP(x)

	#define VER_FILEVERSION             VER_MAJOR,VER_MINOR,VER_PATCH,VER_BUILD
	#if (GitVersion == 0)
	#define VER_FILEVERSION_STR         STR(VER_MAJOR) "." STR(VER_MINOR) "." STR(VER_PATCH) "." STR(VER_BUILD)
	#else
	#define VER_FILEVERSION_STR         STR(VER_MAJOR) "." STR(VER_MINOR) "." STR(VER_PATCH) "." STR(VER_BUILD)
	#endif
	#define VER_COPYRIGHT_STR           "\xA9 2006-2026"

	#define VER_ORIGINAL_FILENAME_STR   "assimp.dll"

	#endif // ASSIMP_REVISION_H_INC
	]])

	file:close()
end

ensure_config_header()
copy_if_missing("contrib/zlib/zconf.h", "contrib/zlib/zconf.h.included")
write_revision_if_missing("include/assimp/revision.h")

project "assimp"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/**.h",
		"include/**.hpp",
		"include/**.inl",

		"code/**.h",
		"code/**.hpp",
		"code/**.inl",
		"code/**.cpp",

		"contrib/clipper/**.h",
		"contrib/clipper/**.hpp",
		"contrib/clipper/**.cpp",
		"contrib/openddlparser/include/**.h",
		"contrib/openddlparser/code/**.h",
		"contrib/openddlparser/code/**.cpp",
		"contrib/poly2tri/**.h",
		"contrib/poly2tri/**.cc",
		"contrib/pugixml/src/**.h",
		"contrib/pugixml/src/**.hpp",
		"contrib/pugixml/src/**.cpp",
		"contrib/unzip/**.h",
		"contrib/unzip/**.c",
		"contrib/zlib/**.h",
		"contrib/zlib/**.c"
	}

	removefiles
	{
		"code/res/assimp.rc",
		"contrib/zlib/contrib/**"
	}

	includedirs
	{
		".",
		"include",
		"code",
		"contrib",
		"contrib/clipper",
		"contrib/openddlparser/include",
		"contrib/poly2tri",
		"contrib/pugixml/src",
		"contrib/rapidjson/include",
		"contrib/utf8cpp/source",
		"contrib/unzip",
		"contrib/zlib"
	}

	defines
	{
		"OPENDDLPARSER_BUILD",
		"OPENDDL_STATIC_LIBARY",
		"P2T_STATIC_EXPORTS",
		"ASSIMP_BUILD_NO_EXPORT",
		"ASSIMP_BUILD_NO_X_IMPORTER",
		"ASSIMP_BUILD_NO_3DS_IMPORTER",
		"ASSIMP_BUILD_NO_MD3_IMPORTER",
		"ASSIMP_BUILD_NO_MDL_IMPORTER",
		"ASSIMP_BUILD_NO_MD2_IMPORTER",
		"ASSIMP_BUILD_NO_ASE_IMPORTER",
		"ASSIMP_BUILD_NO_AMF_IMPORTER",
		"ASSIMP_BUILD_NO_HMP_IMPORTER",
		"ASSIMP_BUILD_NO_SMD_IMPORTER",
		"ASSIMP_BUILD_NO_MDC_IMPORTER",
		"ASSIMP_BUILD_NO_MD5_IMPORTER",
		"ASSIMP_BUILD_NO_STL_IMPORTER",
		"ASSIMP_BUILD_NO_LWO_IMPORTER",
		"ASSIMP_BUILD_NO_DXF_IMPORTER",
		"ASSIMP_BUILD_NO_NFF_IMPORTER",
		"ASSIMP_BUILD_NO_RAW_IMPORTER",
		"ASSIMP_BUILD_NO_OFF_IMPORTER",
		"ASSIMP_BUILD_NO_AC_IMPORTER",
		"ASSIMP_BUILD_NO_BVH_IMPORTER",
		"ASSIMP_BUILD_NO_IRRMESH_IMPORTER",
		"ASSIMP_BUILD_NO_IRR_IMPORTER",
		"ASSIMP_BUILD_NO_Q3D_IMPORTER",
		"ASSIMP_BUILD_NO_B3D_IMPORTER",
		"ASSIMP_BUILD_NO_TERRAGEN_IMPORTER",
		"ASSIMP_BUILD_NO_CSM_IMPORTER",
		"ASSIMP_BUILD_NO_3D_IMPORTER",
		"ASSIMP_BUILD_NO_LWS_IMPORTER",
		"ASSIMP_BUILD_NO_OGRE_IMPORTER",
		"ASSIMP_BUILD_NO_OPENGEX_IMPORTER",
		"ASSIMP_BUILD_NO_MS3D_IMPORTER",
		"ASSIMP_BUILD_NO_COB_IMPORTER",
		"ASSIMP_BUILD_NO_BLEND_IMPORTER",
		"ASSIMP_BUILD_NO_Q3BSP_IMPORTER",
		"ASSIMP_BUILD_NO_NDO_IMPORTER",
		"ASSIMP_BUILD_NO_IFC_IMPORTER",
		"ASSIMP_BUILD_NO_XGL_IMPORTER",
		"ASSIMP_BUILD_NO_ASSBIN_IMPORTER",
		"ASSIMP_BUILD_NO_C4D_IMPORTER",
		"ASSIMP_BUILD_NO_3MF_IMPORTER",
		"ASSIMP_BUILD_NO_X3D_IMPORTER",
		"ASSIMP_BUILD_NO_MMD_IMPORTER",
		"ASSIMP_BUILD_NO_SIB_IMPORTER",
		"ASSIMP_BUILD_NO_IQM_IMPORTER",
		"ASSIMP_BUILD_NO_M3D_IMPORTER",
		"ASSIMP_BUILD_NO_USD_IMPORTER",
		"ASSIMP_BUILD_NO_VRML_IMPORTER",

		"ASSIMP_BUILD_NO_GENFACENORMALS_PROCESS",
		"ASSIMP_BUILD_NO_REMOVEVC_PROCESS",
		"ASSIMP_BUILD_NO_SPLITLARGEMESHES_PROCESS",
		"ASSIMP_BUILD_NO_LIMITBONEWEIGHTS_PROCESS",
		"ASSIMP_BUILD_NO_IMPROVECACHELOCALITY_PROCESS",
		"ASSIMP_BUILD_NO_FIXINFACINGNORMALS_PROCESS",
		"ASSIMP_BUILD_NO_REMOVE_REDUNDANTMATERIALS_PROCESS",
		"ASSIMP_BUILD_NO_FINDINVALIDDATA_PROCESS",
		"ASSIMP_BUILD_NO_FINDDEGENERATES_PROCESS",
		"ASSIMP_BUILD_NO_GENUVCOORDS_PROCESS",
		"ASSIMP_BUILD_NO_TRANSFORMTEXCOORDS_PROCESS",
		"ASSIMP_BUILD_NO_FINDINSTANCES_PROCESS",
		"ASSIMP_BUILD_NO_OPTIMIZEMESHES_PROCESS",
		"ASSIMP_BUILD_NO_OPTIMIZEGRAPH_PROCESS",
		"ASSIMP_BUILD_NO_SPLITBYBONECOUNT_PROCESS",
		"ASSIMP_BUILD_NO_DEBONE_PROCESS",
		"ASSIMP_BUILD_NO_EMBEDTEXTURES_PROCESS",
		"ASSIMP_BUILD_NO_GLOBALSCALE_PROCESS"
	}

	warnings "off"

	filter "system:windows"
		systemversion "latest"
		staticruntime "off"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		staticruntime "off"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		-- Importing a multi-million-vertex mesh through an unoptimized assimp takes
		-- minutes (CalcTangentsProcess dominates). We never step into assimp, so
		-- optimize it even in Debug; runtime stays "Debug" to match the CRT the rest
		-- of the solution links against.
		optimize "on"
		runtimechecks "off"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"
