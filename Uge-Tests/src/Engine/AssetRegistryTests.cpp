/**
 * @file AssetRegistryTests.cpp
 * @brief Tests for Uge::EditorAssetManager and the asset type helpers.
 */

#include <ugpch.h>

#include "TestSupport/MockTexture2D.h"
#include "TestSupport/TempProject.h"

#include "Uge/Asset/AssetManager.h"
#include "Uge/Asset/EditorAssetManager.h"
#include "Uge/Project/Project.h"
#include "Uge/Scene/Entity.h"
#include "Uge/Scene/Scene.h"
#include "Uge/Scene/SceneSerializer.h"

#include <gtest/gtest.h>

using Uge::AssetHandle;
using Uge::AssetManager;
using Uge::AssetType;
using Uge::EditorAssetManager;
using Uge::Project;
using Uge::Scene;
using UgeTests::TempProject;

namespace
{

	/**
	 * @brief A project with an asset manager. Only scenes can be imported without a GPU.
	 */
	class AssetRegistryTest : public ::testing::Test
	{
	protected:
		EditorAssetManager& Manager() const
		{
			return *Project::GetActive()->GetEditorAssetManager();
		}

		/**
		 * @brief Writes a scene file in the asset directory.
		 * @param relativePath Path relative to the asset directory, ending in `.uge`.
		 * @return The same relative path.
		 */
		std::filesystem::path WriteScene(const std::filesystem::path& relativePath) const
		{
			Uge::Ref<Scene> scene = Uge::CreateRef<Scene>();
			scene->CreateEntity("Entity");

			// SceneSerializer doesn't create the directory.
			const std::filesystem::path absolute = Project::GetAssetFileSystemPath(relativePath);
			std::filesystem::create_directories(absolute.parent_path());

			Uge::SceneSerializer serializer(scene);
			serializer.Serialize(absolute);

			return relativePath;
		}

		TempProject m_project;
	};

}

TEST(AssetTypeTest, RoundTripsThroughItsSerializedName)
{
	// The registry stores these names, so renaming one breaks existing registries.
	for (AssetType type : { AssetType::Scene, AssetType::Texture2D, AssetType::Mesh,
		AssetType::Material, AssetType::Environment })
	{
		EXPECT_EQ(Uge::AssetTypeFromString(Uge::AssetTypeToString(type)), type);
	}
}

TEST(AssetTypeTest, UnknownNameBecomesNone)
{
	EXPECT_EQ(Uge::AssetTypeFromString("NotAnAssetType"), AssetType::None);
	EXPECT_EQ(Uge::AssetTypeFromString(""), AssetType::None);
}

TEST_F(AssetRegistryTest, StartsWithAnEmptyRegistry)
{
	EXPECT_TRUE(Manager().GetAssetRegistry().empty());
}

TEST_F(AssetRegistryTest, MissingRegistryFileIsCreatedRatherThanFailingTheLoad)
{
	// A project without a registry file still opens.
	EXPECT_TRUE(std::filesystem::exists(Project::GetAssetRegistryPath()));
}

TEST_F(AssetRegistryTest, ZeroIsNeverAValidHandle)
{
	// 0 means no asset.
	EXPECT_FALSE(AssetManager::IsAssetHandleValid(0));
	EXPECT_EQ(AssetManager::GetAssetType(0), AssetType::None);
	EXPECT_EQ(AssetManager::GetAsset<Scene>(0), nullptr);
}

TEST_F(AssetRegistryTest, UnknownHandleResolvesToNothing)
{
	const AssetHandle stranger(0xFEEDFACE);

	EXPECT_FALSE(AssetManager::IsAssetHandleValid(stranger));
	EXPECT_FALSE(Manager().GetMetadata(stranger));
	EXPECT_TRUE(Manager().GetFilePath(stranger).empty());
}

TEST_F(AssetRegistryTest, ImportingRegistersTypeAndPath)
{
	const std::filesystem::path relative = WriteScene("Scenes/Level.uge");

	const AssetHandle handle = Manager().ImportAsset(relative, "Level");

	ASSERT_NE((uint64_t)handle, 0ull);
	EXPECT_TRUE(Manager().IsAssetHandleValid(handle));
	EXPECT_EQ(Manager().GetAssetType(handle), AssetType::Scene);
	EXPECT_EQ(Manager().GetFilePath(handle), relative);
	EXPECT_TRUE(Manager().IsAssetLoaded(handle));
}

TEST_F(AssetRegistryTest, ImportedAssetResolvesToItsContents)
{
	Uge::Ref<Scene> source = Uge::CreateRef<Scene>();
	source->CreateEntity("Imported");

	const std::filesystem::path absolute = Project::GetAssetFileSystemPath("Scenes/One.uge");
	std::filesystem::create_directories(absolute.parent_path());
	Uge::SceneSerializer(source).Serialize(absolute);

	const AssetHandle handle = Manager().ImportAsset("Scenes/One.uge", "One");

	Uge::Ref<Scene> loaded = AssetManager::GetAsset<Scene>(handle);
	ASSERT_NE(loaded, nullptr);
	EXPECT_TRUE(loaded->FindEntityByName("Imported"));
}

TEST_F(AssetRegistryTest, ImportingTheSameFileTwiceCreatesTwoAssets)
{
	// ImportAsset always makes a new handle; GetOrImportAsset doesn't.
	const std::filesystem::path relative = WriteScene("Scenes/Level.uge");

	const AssetHandle first = Manager().ImportAsset(relative);
	const AssetHandle second = Manager().ImportAsset(relative);

	EXPECT_NE((uint64_t)first, (uint64_t)second);
	EXPECT_EQ(Manager().GetAssetRegistry().size(), 2u);
}

TEST_F(AssetRegistryTest, GetOrImportReusesTheExistingHandle)
{
	const std::filesystem::path relative = WriteScene("Scenes/Level.uge");

	const AssetHandle first = Manager().GetOrImportAsset(relative);
	const AssetHandle second = Manager().GetOrImportAsset(relative);

	EXPECT_EQ((uint64_t)first, (uint64_t)second);
	EXPECT_EQ(Manager().GetAssetRegistry().size(), 1u);
}

TEST_F(AssetRegistryTest, RegistrySurvivesAReload)
{
	// Handles survive closing and reopening the project.
	const std::filesystem::path relative = WriteScene("Scenes/Level.uge");
	const AssetHandle handle = Manager().ImportAsset(relative, "Level");

	Manager().SerializeAssetRegistry();

	ASSERT_TRUE(Project::Load(m_project.ProjectFile()));

	EXPECT_TRUE(Manager().IsAssetHandleValid(handle));
	EXPECT_EQ(Manager().GetAssetType(handle), AssetType::Scene);
	EXPECT_EQ(Manager().GetFilePath(handle), relative);

	// Registered, but not loaded.
	EXPECT_FALSE(Manager().IsAssetLoaded(handle));
}

TEST_F(AssetRegistryTest, MemoryOnlyAssetsAreUsableButNotRegistered)
{
	// As for the materials made during mesh import, which have no file.
	Uge::Ref<UgeTests::MockTexture2D> texture = Uge::CreateRef<UgeTests::MockTexture2D>();

	const AssetHandle handle = Manager().AddMemoryOnlyAsset(texture);

	EXPECT_TRUE(Manager().IsAssetHandleValid(handle));
	EXPECT_TRUE(Manager().IsAssetLoaded(handle));
	EXPECT_EQ(Manager().GetAssetType(handle), AssetType::Texture2D);
	EXPECT_EQ(AssetManager::GetAsset<Uge::Texture2D>(handle), texture);

	EXPECT_TRUE(Manager().GetAssetRegistry().empty());
	EXPECT_FALSE(Manager().GetMetadata(handle));
}

TEST_F(AssetRegistryTest, LoadedAssetOfAnotherTypeResolvesToNull)
{
	// Asking for an asset as the wrong type gives null.
	Uge::Ref<UgeTests::MockTexture2D> texture = Uge::CreateRef<UgeTests::MockTexture2D>();
	const AssetHandle handle = Manager().AddMemoryOnlyAsset(texture);

	EXPECT_EQ(AssetManager::GetAsset<Scene>(handle), nullptr);
	EXPECT_EQ(AssetManager::GetAsset<Uge::Texture2D>(handle), texture);
}

TEST_F(AssetRegistryTest, RegisteredAssetOfAnotherTypeResolvesToNull)
{
	// The same for an asset not yet loaded, which then still loads as its own type.
	const AssetHandle handle = Manager().ImportAsset(WriteScene("Scenes/NotATexture.uge"), "NotATexture");

	EXPECT_EQ(AssetManager::GetAsset<Uge::Texture2D>(handle), nullptr);
	EXPECT_NE(AssetManager::GetAsset<Scene>(handle), nullptr);
}

TEST_F(AssetRegistryTest, MemoryOnlyAssetsAreNotWrittenToTheRegistryFile)
{
	Manager().AddMemoryOnlyAsset(Uge::CreateRef<UgeTests::MockTexture2D>());
	Manager().SerializeAssetRegistry();

	ASSERT_TRUE(Project::Load(m_project.ProjectFile()));

	EXPECT_TRUE(Manager().GetAssetRegistry().empty());
}

TEST_F(AssetRegistryTest, RegisteringAnAssetStampsItsHandleAndName)
{
	Uge::Ref<UgeTests::MockTexture2D> texture = Uge::CreateRef<UgeTests::MockTexture2D>();

	const AssetHandle handle = Manager().AddMemoryOnlyAsset(texture);

	EXPECT_EQ((uint64_t)texture->m_handle, (uint64_t)handle);
}

TEST_F(AssetRegistryTest, LoadedAssetNamesArePrefixedWithTheirType)
{
	const std::filesystem::path relative = WriteScene("Scenes/Level.uge");
	Manager().ImportAsset(relative, "Level");

	const std::vector<std::string> names = Manager().GetLoadedAssetsNames();

	ASSERT_EQ(names.size(), 1u);

	// SetName prefixes the name with AssetTypeToString's, AssetType::Scene.
	EXPECT_EQ(names[0], "AssetType::Scene - Level");
}

TEST_F(AssetRegistryTest, MeshMetadataRoundTripsThroughTheMeshRegistry)
{
	const AssetHandle model(0xA11CE);
	const AssetHandle material(0xB0B);
	const AssetHandle texture(0xC0FFEE);

	Uge::MeshAssetMetadata metadata;
	metadata.MaterialHandles = { material };
	metadata.Dependencies = { material, texture };

	Manager().SetMeshMetadata(model, metadata);

	EXPECT_TRUE(Manager().IsMeshAssetHandleValid(model));

	const Uge::MeshAssetMetadata& stored = Manager().GetMeshMetadata(model);
	ASSERT_EQ(stored.MaterialHandles.size(), 1u);
	EXPECT_EQ((uint64_t)stored.MaterialHandles[0], (uint64_t)material);
	EXPECT_EQ(stored.Dependencies.size(), 2u);
}

TEST_F(AssetRegistryTest, UnknownMeshHandleHasNoMetadata)
{
	EXPECT_FALSE(Manager().IsMeshAssetHandleValid(AssetHandle(0xDEAD)));
	EXPECT_TRUE(Manager().GetMeshMetadata(AssetHandle(0xDEAD)).MaterialHandles.empty());
}

TEST_F(AssetRegistryTest, DeletingAModelDropsItsMeshEntryAndUnloadsIt)
{
	Uge::Ref<UgeTests::MockTexture2D> stand_in = Uge::CreateRef<UgeTests::MockTexture2D>();
	const AssetHandle model = Manager().AddMemoryOnlyAsset(stand_in);

	Manager().SetMeshMetadata(model, Uge::MeshAssetMetadata{});

	Manager().DeleteModel(model);

	EXPECT_FALSE(Manager().IsMeshAssetHandleValid(model));
	EXPECT_FALSE(Manager().IsAssetLoaded(model));
}

TEST_F(AssetRegistryTest, DeletingAnAssetUnloadsItButKeepsItRegistered)
{
	// Unloading keeps the asset registered, so it imports again on next use.
	const std::filesystem::path relative = WriteScene("Scenes/Level.uge");
	const AssetHandle handle = Manager().ImportAsset(relative, "Level");

	Uge::Ref<UgeTests::MockTexture2D> texture = Uge::CreateRef<UgeTests::MockTexture2D>();
	const AssetHandle textureHandle = Manager().AddMemoryOnlyAsset(texture);

	Manager().DeleteAsset(textureHandle);

	EXPECT_FALSE(Manager().IsAssetLoaded(textureHandle));

	// The scene is still registered and can be resolved again.
	EXPECT_TRUE(Manager().IsAssetHandleValid(handle));
	EXPECT_NE(AssetManager::GetAsset<Scene>(handle), nullptr);
}

TEST_F(AssetRegistryTest, DeletingASceneUnloadsItSoTheFileIsReadAgain)
{
	// After DeleteAsset, GetAsset reads the saved file instead of the cached scene.
	const std::filesystem::path relative = WriteScene("Scenes/Level.uge");
	const AssetHandle handle = Manager().ImportAsset(relative, "Level");
	ASSERT_NE(AssetManager::GetAsset<Scene>(handle), nullptr);

	// Save a changed scene over the file, as the editor does.
	Uge::Ref<Scene> edited = Uge::CreateRef<Scene>();
	edited->CreateEntity("Added in the editor");
	Uge::SceneSerializer(edited).Serialize(Project::GetAssetFileSystemPath(relative));

	Manager().DeleteAsset(handle);
	EXPECT_FALSE(Manager().IsAssetLoaded(handle));

	Uge::Ref<Scene> reloaded = AssetManager::GetAsset<Scene>(handle);
	ASSERT_NE(reloaded, nullptr);
	EXPECT_TRUE(reloaded->FindEntityByName("Added in the editor"));
}

TEST_F(AssetRegistryTest, ResolvingAnAssetWhoseFileIsGoneReturnsNull)
{
	// A registered asset whose file is gone resolves to null without throwing.
	const std::filesystem::path relative = WriteScene("Scenes/Level.uge");
	const AssetHandle handle = Manager().ImportAsset(relative, "Level");
	Manager().SerializeAssetRegistry();

	// Reopen, so nothing is cached, then delete the file.
	ASSERT_TRUE(Project::Load(m_project.ProjectFile()));
	ASSERT_TRUE(std::filesystem::remove(Project::GetAssetFileSystemPath(relative)));

	Uge::Ref<Scene> resolved;
	EXPECT_NO_THROW({ resolved = AssetManager::GetAsset<Scene>(handle); });

	// Null, not an empty scene that the editor could save over the missing file.
	EXPECT_EQ(resolved, nullptr);
}

TEST_F(AssetRegistryTest, ResolvingAMalformedAssetReturnsNull)
{
	const std::filesystem::path relative = WriteScene("Scenes/Level.uge");
	const AssetHandle handle = Manager().ImportAsset(relative, "Level");
	Manager().SerializeAssetRegistry();

	ASSERT_TRUE(Project::Load(m_project.ProjectFile()));

	{
		std::ofstream out(Project::GetAssetFileSystemPath(relative), std::ios::trunc);
		out << "Scene: [unterminated\n  - nonsense: {\n";
	}

	Uge::Ref<Scene> resolved;
	EXPECT_NO_THROW({ resolved = AssetManager::GetAsset<Scene>(handle); });
	EXPECT_EQ(resolved, nullptr);
}

TEST_F(AssetRegistryTest, ReleasedAssetsAreReimportedOnNextUse)
{
	const std::filesystem::path relative = WriteScene("Scenes/Level.uge");
	const AssetHandle handle = Manager().ImportAsset(relative, "Level");

	Uge::Ref<Scene> first = AssetManager::GetAsset<Scene>(handle);
	ASSERT_NE(first, nullptr);

	EXPECT_EQ(AssetManager::GetAsset<Scene>(handle), first);
}
