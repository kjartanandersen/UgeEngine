#include <ugpch.h>
#include "TestSupport/TempProject.h"

#include "Uge/Core/UUID.h"

#include <fstream>

namespace UgeTests
{

	TempProject::TempProject(const std::string& name)
	{
		// A UUID rather than the test name: two fixtures alive at once, or a previous run
		// that failed to clean up, must not collide.
		m_root = std::filesystem::temp_directory_path()
			/ ("uge-tests-" + std::to_string((uint64_t)Uge::UUID()));

		std::filesystem::create_directories(AssetDirectory());

		Uge::Ref<Uge::Project> project = Uge::Project::New();

		Uge::ProjectConfig& config = project->GetConfig();
		config.Name = name;
		config.AssetDirectory = "Assets";
		config.AssetRegistryPath = "AssetRegistry.ugreg";
		config.ScriptModulePath = "Scripts/Binaries/TestScripts.dll";

		m_projectFile = m_root / (name + ".ugproj");

		// Save then load, rather than assigning the singleton directly: Load() is what
		// creates the EditorAssetManager and reads the registry, so this is the same path
		// the editor takes when it opens a project.
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
