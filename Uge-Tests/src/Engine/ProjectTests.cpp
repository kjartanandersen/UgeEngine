/**
 * @file ProjectTests.cpp
 * @brief Tests for Uge::Project and its YAML serializer.
 */

#include <ugpch.h>

#include "TestSupport/TempProject.h"

#include "Uge/Project/Project.h"
#include "Uge/Project/ProjectSerializer.h"

#include <gtest/gtest.h>

#include <fstream>
#include <type_traits>
#include <utility>

// The asset facade calls these on every lookup, so they return references.
static_assert(std::is_reference_v<decltype(Uge::Project::GetActive())>);
static_assert(std::is_reference_v<decltype(std::declval<const Uge::Project&>().GetAssetManager())>);

using Uge::Project;
using Uge::ProjectConfig;
using UgeTests::TempProject;

namespace
{

	/** @brief A new active project for each test. */
	class ProjectTest : public ::testing::Test
	{
	protected:
		TempProject m_project;

		/** @brief Writes a `.ugproj` file in the project root and returns its path. */
		std::filesystem::path WriteProjectFile(const std::string& name, const std::string& contents) const
		{
			const std::filesystem::path path = m_project.Root() / name;
			std::ofstream out(path);
			out << contents;
			return path;
		}
	};

}

TEST_F(ProjectTest, LoadingMakesTheProjectActive)
{
	ASSERT_TRUE(Project::GetActive());
	EXPECT_EQ(Project::GetActive()->GetConfig().Name, "TestProject");
}

TEST_F(ProjectTest, LoadingCreatesAnEditorAssetManager)
{
	// Only Load() creates the asset manager, not New().
	ASSERT_TRUE(Project::GetActive());
	EXPECT_NE(Project::GetActive()->GetAssetManager(), nullptr);
	EXPECT_NE(Project::GetActive()->GetEditorAssetManager(), nullptr);
}

TEST_F(ProjectTest, ProjectDirectoryIsTheFoldersHoldingTheProjectFile)
{
	EXPECT_EQ(Project::GetProjectDirectory(), m_project.Root());
}

TEST_F(ProjectTest, AssetDirectoryIsResolvedAgainstTheProjectDirectory)
{
	EXPECT_EQ(Project::GetAssetDirectory(), m_project.AssetDirectory());
}

TEST_F(ProjectTest, AssetRegistryPathSitsInsideTheAssetDirectory)
{
	EXPECT_EQ(Project::GetAssetRegistryPath(), m_project.AssetDirectory() / "AssetRegistry.ugreg");
}

TEST_F(ProjectTest, AssetFileSystemPathResolvesAgainstTheContentRoot)
{
	EXPECT_EQ(Project::GetAssetFileSystemPath("Scenes/Level.uge"),
		m_project.AssetDirectory() / "Scenes/Level.uge");
}

TEST_F(ProjectTest, RelativePathIsTheInverseOfAssetFileSystemPath)
{
	// The two conversions undo each other.
	const std::filesystem::path absolute = Project::GetAssetFileSystemPath("Textures/Icon.png");

	EXPECT_EQ(Project::GetRelativePath(absolute.string()),
		std::filesystem::path("Textures/Icon.png"));
}

TEST_F(ProjectTest, RelativePathStringMatchesRelativePath)
{
	const std::filesystem::path absolute = Project::GetAssetFileSystemPath("Meshes/Cube.obj");

	EXPECT_EQ(Project::GetRelativePathString(absolute.string()),
		Project::GetRelativePath(absolute.string()).string());
}

TEST_F(ProjectTest, UnloadReturnsToTheNoProjectState)
{
	ASSERT_TRUE(Project::GetActive());

	Project::Unload();

	EXPECT_EQ(Project::GetActive(), nullptr);

	// Loaded again for the fixture's destructor.
	Project::Load(m_project.ProjectFile());
	EXPECT_TRUE(Project::GetActive());
}

TEST_F(ProjectTest, ConfigurationSurvivesASaveAndLoadRoundTrip)
{
	ProjectConfig& config = Project::GetActive()->GetConfig();
	config.Name = "Renamed";
	config.StartScene = Uge::AssetHandle(0x1234ABCD);
	config.AssetDirectory = "Content";
	config.AssetRegistryPath = "Registry.ugreg";
	config.ScriptModulePath = "Binaries/Game.dll";

	const std::filesystem::path saved = m_project.Root() / "RoundTrip.ugproj";
	ASSERT_TRUE(Project::SaveActive(saved));

	ASSERT_TRUE(Project::Load(saved));

	const ProjectConfig& loaded = Project::GetActive()->GetConfig();
	EXPECT_EQ(loaded.Name, "Renamed");
	EXPECT_EQ((uint64_t)loaded.StartScene, 0x1234ABCDull);
	EXPECT_EQ(loaded.AssetDirectory, std::filesystem::path("Content"));
	EXPECT_EQ(loaded.AssetRegistryPath, std::filesystem::path("Registry.ugreg"));
	EXPECT_EQ(loaded.ScriptModulePath, std::filesystem::path("Binaries/Game.dll"));
}

TEST_F(ProjectTest, SavingWritesAReadableProjectFile)
{
	const std::filesystem::path saved = m_project.Root() / "Written.ugproj";

	ASSERT_TRUE(Project::SaveActive(saved));
	ASSERT_TRUE(std::filesystem::exists(saved));
	EXPECT_GT(std::filesystem::file_size(saved), 0u);
}

TEST_F(ProjectTest, SavingElsewhereMovesTheProjectDirectory)
{
	// Saving elsewhere moves the project directory with it.
	const std::filesystem::path nested = m_project.Root() / "Nested";
	std::filesystem::create_directories(nested);

	ASSERT_TRUE(Project::SaveActive(nested / "Moved.ugproj"));

	EXPECT_EQ(Project::GetProjectDirectory(), nested);
}

TEST_F(ProjectTest, DeserializingAMalformedProjectFails)
{
	const std::filesystem::path bad = m_project.Root() / "Malformed.ugproj";
	{
		std::ofstream out(bad);
		out << "NotAProject:\n  key: value\n";
	}

	Uge::Ref<Uge::Project> target = Uge::CreateRef<Uge::Project>();
	Uge::ProjectSerializer serializer(target);

	EXPECT_FALSE(serializer.Deserialize(bad));
}

TEST_F(ProjectTest, LoadingAMissingFileReturnsNull)
{
	// A missing file fails without throwing YAML::BadFile, which isn't a ParserException.
	EXPECT_NO_THROW({
		EXPECT_EQ(Project::Load(m_project.Root() / "DoesNotExist.ugproj"), nullptr);
		});

	// The open project stays open.
	ASSERT_TRUE(Project::GetActive());
	EXPECT_EQ(Project::GetActive()->GetConfig().Name, "TestProject");
	EXPECT_EQ(Project::GetProjectDirectory(), m_project.Root());
}

TEST_F(ProjectTest, LoadingADirectoryReturnsNull)
{
	// An unreadable file also raises YAML::BadFile.
	EXPECT_NO_THROW({
		EXPECT_EQ(Project::Load(m_project.AssetDirectory()), nullptr);
		});

	EXPECT_TRUE(Project::GetActive());
}

TEST_F(ProjectTest, ANewProjectHasNoStartScene)
{
	// The fixture saved and loaded the project, so 0 also survives a round trip.
	EXPECT_EQ((uint64_t)Project::GetActive()->GetConfig().StartScene, 0ull);
}

TEST_F(ProjectTest, AStartSceneThatIsNotAHandleIsIgnored)
{
	// A path in place of a handle: the rest loads, with no start scene.
	const std::filesystem::path file = WriteProjectFile("PathStartScene.ugproj",
		"Project:\n"
		"  Name: Sample\n"
		"  StartScene: \"Scenes/Level.uge\"\n"
		"  AssetDirectory: Assets\n"
		"  ScriptModulePath: Scripts/Binaries/Sample.dll\n");

	Uge::Ref<Project> target = Uge::CreateRef<Project>();
	Uge::ProjectSerializer serializer(target);

	bool loaded = false;
	EXPECT_NO_THROW(loaded = serializer.Deserialize(file));
	EXPECT_TRUE(loaded);
	EXPECT_EQ((uint64_t)target->GetConfig().StartScene, 0ull);
	EXPECT_EQ(target->GetConfig().Name, "Sample");
}

TEST_F(ProjectTest, AProjectFileMissingARequiredSettingFails)
{
	const char* lines[] = {
		"  Name: Sample\n",
		"  AssetDirectory: Assets\n",
		"  ScriptModulePath: Scripts/Binaries/Sample.dll\n",
	};

	for (int missing = 0; missing < 3; missing++)
	{
		std::string contents = "Project:\n";
		for (int i = 0; i < 3; i++)
		{
			if (i != missing)
			{
				contents += lines[i];
			}
		}
		const std::filesystem::path file = WriteProjectFile("Missing" + std::to_string(missing) + ".ugproj", contents);

		Uge::Ref<Project> target = Uge::CreateRef<Project>();
		Uge::ProjectSerializer serializer(target);

		bool loaded = true;
		EXPECT_NO_THROW(loaded = serializer.Deserialize(file));
		EXPECT_FALSE(loaded) << "Loaded without:" << lines[missing];
	}
}

TEST_F(ProjectTest, AMistypedSettingFailsWithoutThrowing)
{
	// A sequence in place of the name.
	const std::filesystem::path file = WriteProjectFile("Mistyped.ugproj",
		"Project:\n"
		"  Name: [a, b]\n"
		"  AssetDirectory: Assets\n"
		"  ScriptModulePath: Scripts/Binaries/Sample.dll\n");

	Uge::Ref<Project> target = Uge::CreateRef<Project>();
	Uge::ProjectSerializer serializer(target);

	bool loaded = true;
	EXPECT_NO_THROW(loaded = serializer.Deserialize(file));
	EXPECT_FALSE(loaded);
	EXPECT_EQ(target->GetConfig().Name, "Untitled");
}

TEST_F(ProjectTest, AFileThatIsNotAMapFailsWithoutThrowing)
{
	// yaml-cpp throws when a scalar document is subscripted.
	const std::filesystem::path file = WriteProjectFile("Scalar.ugproj", "just some text\n");

	Uge::Ref<Project> target = Uge::CreateRef<Project>();
	Uge::ProjectSerializer serializer(target);

	bool loaded = true;
	EXPECT_NO_THROW(loaded = serializer.Deserialize(file));
	EXPECT_FALSE(loaded);
}

TEST_F(ProjectTest, SavingIntoAMissingFolderFails)
{
	EXPECT_FALSE(Project::SaveActive(m_project.Root() / "NoSuchFolder" / "TestProject.ugproj"));

	// The project directory doesn't change.
	EXPECT_EQ(Project::GetProjectDirectory(), m_project.Root());
}

