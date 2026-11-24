#include <ugpch.h>
#include "TestSupport/TempProject.h"

#include "Uge/Core/UUID.h"

#include <fstream>

namespace UgeTests
{

	TempProject::TempProject(const std::string& name)
	{
		// A UUID, so it can't collide with another fixture or a previous run's leftovers.
		m_root = std::filesystem::temp_directory_path()
			/ ("uge-tests-" + std::to_string((uint64_t)Uge::UUID((uint64_t)Uge::UUID::Generate())));

		std::filesystem::create_directories(AssetDirectory());

		Uge::Ref<Uge::Project> project = Uge::Project::New();

		Uge::ProjectConfig& config = project->GetConfig();
		config.Name = name;
		config.AssetDirectory = "Assets";
		config.AssetRegistryPath = "AssetRegistry.ugreg";
		config.ScriptModulePath = "Scripts/Binaries/TestScripts.dll";

		m_projectFile = m_root / (name + ".ugproj");

		// Saved and loaded, as the editor does: Load() creates the asset manager.
		Uge::Project::SaveActive(m_projectFile);
		Uge::Project::Load(m_projectFile);
	}

	TempProject::~TempProject()
	{
		Uge::Project::Unload();

		std::error_code error;
		std::filesystem::remove_all(m_root, error);
	}

	std::filesystem::path TempProject::WriteAssetFile(const std::filesystem::path& relativePath,
		const std::string& contents) const
	{
		const std::filesystem::path absolute = AssetDirectory() / relativePath;

		std::filesystem::create_directories(absolute.parent_path());

		std::ofstream out(absolute);
		out << contents;

		return absolute;
	}

}
