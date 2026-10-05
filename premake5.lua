include "./thirdparty/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

-- Seven dependency build scripts can't be committed inside their submodules, because those
-- point at the upstream repositories. The versioned copies in premake/thirdparty/ are the
-- source of truth, and are put in place here, before the includes below need them. The same
-- goes for local fixes to dependency sources, kept in patches/. See premake/thirdparty/README.md.
local thirdPartyScripts = { "GLFW", "imgui", "glm", "yaml-cpp", "JoltPhysics", "googletest", "assimp" }

-- core.autocrlf can give either copy different line endings, which is not a real difference.
local function NormalizedText(file)
	local text = io.readfile(file)
	return text and (text:gsub("\r\n", "\n"))
end

local function SyncThirdPartyScripts()
	for _, name in ipairs(thirdPartyScripts) do
		local dir    = path.join(_MAIN_SCRIPT_DIR, "Uge/thirdparty", name)
		local source = path.join(_MAIN_SCRIPT_DIR, "premake/thirdparty", name, "premake5.lua")
		local target = path.join(dir, "premake5.lua")

		if not os.isfile(path.join(dir, ".git")) then
			-- Copying into an empty, uninitialised submodule would only fail later, and more confusingly.
			error(dir .. " is not checked out. Run: git submodule update --init --recursive", 0)
		elseif not os.isfile(target) then
			assert(os.copyfile(source, target))
			print("Restored " .. target)
		elseif NormalizedText(source) ~= NormalizedText(target) then
			-- Never overwrite: the submodule copy may hold an edit that hasn't been moved over yet.
			error(target .. " differs from " .. source .. ".\n" ..
				"Make the change in premake/thirdparty/, delete the submodule copy, and re-run.", 0)
		end
	end
end

local function ApplyThirdPartyPatches()
	local assimp = path.join(_MAIN_SCRIPT_DIR, "Uge/thirdparty/assimp")
	local patch  = path.join(_MAIN_SCRIPT_DIR, "patches/assimp-fbx-rotation-loop.patch")

	local function GitApply(options)
		local _, exitCode = os.outputof(string.format('git -C "%s" apply %s "%s"', assimp, options, patch))
		return exitCode == 0
	end

	local _, gitExitCode = os.outputof("git --version")
	if gitExitCode ~= 0 then
		error("git was not found on PATH; it is needed to apply " .. patch, 0)
	end

	if GitApply("--check --reverse") then
		return	-- already applied
	end
	if not GitApply("--check") then
		error(patch .. " no longer applies to assimp. Update it, or delete it if assimp has fixed the bug upstream.", 0)
	end
	assert(GitApply(""), "git apply failed for " .. patch)
	print("Applied " .. patch)
end

SyncThirdPartyScripts()
ApplyThirdPartyPatches()

workspace "Uge"
	architecture "x64"
	startproject "Uge-Editor"
	
	configurations 
	{
		"Debug",
		"Release",
		"Dist"
	}

		solution_items
	{
		".editorconfig"
	}

	multiprocessorcompile ("on")

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


group "Dependencies"
	include "thirdparty/premake"
	include "Uge/thirdparty/GLFW"
	include "Uge/thirdparty/GLAD"
	include "Uge/thirdparty/imgui"
	include "Uge/thirdparty/glm"
	include "Uge/thirdparty/ch-msdf-atlas-gen"
	include "Uge/thirdparty/assimp"
	include "Uge/thirdparty/yaml-cpp"
	include "Uge/thirdparty/JoltPhysics"
	include "Uge/thirdparty/googletest"

group ""

group "Core"
	include "Uge"
	include "Uge-ScriptCore"
group ""

group "Tools"
	include "UgeEditor"
group ""

group "Tests"
	include "Uge-Tests"
group ""

group "Misc"
	include "Sandbox"
group ""

