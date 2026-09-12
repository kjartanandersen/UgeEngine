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

using Uge::Project;
using Uge::ProjectConfig;
using UgeTests::TempProject;

namespace
{

	/** @brief Gives each test a freshly created project, active for its duration. */
	class ProjectTest : public ::testing::Test
	{
	protected:
		TempProject m_project;
	};

}

TEST_F(ProjectTest, LoadingMakesTheProjectActive)
{
	ASSERT_TRUE(Project::GetActive());
	EXPECT_EQ(Project::GetActive()->GetConfig().Name, "TestProject");
}

TEST_F(ProjectTest, LoadingCreatesAnEditorAssetManager)
{
	// Project::New() alone does not; only the load path builds one, which is why the
	// fixture goes through a real save and load.
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
	// The registry stores relative paths and the content browser hands back absolute ones,
	// so these two have to compose back to the identity or a dragged asset registers under
	// a path nothing can resolve.
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

	// Restored so the fixture's destructor and any following test see a consistent state.
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
	// Save As has to repoint the content root, or the project keeps resolving assets
	// against the folder it was opened from.
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
	// A stale path — a recent-projects entry pointing at something that has been moved —
	// has to come back as a null return, not as an exception thrown through Load. That
	// needs the catch in ProjectSerializer::Deserialize to cover YAML::BadFile, which is
	// not a ParserException.
	EXPECT_NO_THROW({
		EXPECT_EQ(Project::Load(m_project.Root() / "DoesNotExist.ugproj"), nullptr);
		});

	// A failed load must leave the project that was open alone.
	ASSERT_TRUE(Project::GetActive());
	EXPECT_EQ(Project::GetActive()->GetConfig().Name, "TestProject");
	EXPECT_EQ(Project::GetProjectDirectory(), m_project.Root());
}

TEST_F(ProjectTest, LoadingADirectoryReturnsNull)
{
	// Not a missing file but an unreadable one, which raises YAML::BadFile just the same.
	EXPECT_NO_THROW({
		EXPECT_EQ(Project::Load(m_project.AssetDirectory()), nullptr);
		});

	EXPECT_TRUE(Project::GetActive());
}
