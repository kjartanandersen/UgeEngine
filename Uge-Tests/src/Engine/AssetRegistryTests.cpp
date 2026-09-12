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
	 * @brief A project with an asset manager, plus a helper for making importable assets.
	 *
	 * Scenes are the only asset type that can be imported without a GPU: the texture, mesh
	 * and environment importers all end up calling into OpenGL. So a `.uge` file is what
	 * these tests register when they need a real registry entry.
	 */
	class AssetRegistryTest : public ::testing::Test
	{
	protected:
		EditorAssetManager& Manager() const
		{
			return *Project::GetActive()->GetEditorAssetManager();
		}

		/**
		 * @brief Writes a real scene file into the asset directory.
		 * @param relativePath Path relative to the asset directory, ending in `.uge`.
		 * @return The same relative path, ready to hand to ImportAsset.
		 */
		std::filesystem::path WriteScene(const std::filesystem::path& relativePath) const
		{
			Uge::Ref<Scene> scene = Uge::CreateRef<Scene>();
			scene->CreateEntity("Entity");

			// SceneSerializer opens an ofstream without creating the directory first, so a
			// missing folder would leave no file behind and no error either.
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
	// The registry file stores these as text, so a rename here silently invalidates every
	// existing project's registry.
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
	// A project checked out without its registry still has to open.
	EXPECT_TRUE(std::filesystem::exists(Project::GetAssetRegistryPath()));
}

TEST_F(AssetRegistryTest, ZeroIsNeverAValidHandle)
{
	// Components use 0 for "nothing assigned", so it must not resolve to an asset.
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
	// Documented behaviour, and the reason GetOrImportAsset exists.
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
	// Handles are what scenes store, so they have to mean the same thing after the project
	// is closed and opened again.
	const std::filesystem::path relative = WriteScene("Scenes/Level.uge");
	const AssetHandle handle = Manager().ImportAsset(relative, "Level");

	Manager().SerializeAssetRegistry();

	ASSERT_TRUE(Project::Load(m_project.ProjectFile()));

	EXPECT_TRUE(Manager().IsAssetHandleValid(handle));
	EXPECT_EQ(Manager().GetAssetType(handle), AssetType::Scene);
	EXPECT_EQ(Manager().GetFilePath(handle), relative);

	// Reloaded from the registry means registered but not yet in memory.
	EXPECT_FALSE(Manager().IsAssetLoaded(handle));
}

TEST_F(AssetRegistryTest, MemoryOnlyAssetsAreUsableButNotRegistered)
{
	// Materials synthesized during mesh import take this path: no file to reimport from,
	// so writing them to the registry would leave entries nothing could resolve.
	Uge::Ref<UgeTests::MockTexture2D> texture = Uge::CreateRef<UgeTests::MockTexture2D>();

	const AssetHandle handle = Manager().AddMemoryOnlyAsset(texture);

	EXPECT_TRUE(Manager().IsAssetHandleValid(handle));
	EXPECT_TRUE(Manager().IsAssetLoaded(handle));
	EXPECT_EQ(Manager().GetAssetType(handle), AssetType::Texture2D);
	EXPECT_EQ(AssetManager::GetAsset<Uge::Texture2D>(handle), texture);

	EXPECT_TRUE(Manager().GetAssetRegistry().empty());
	EXPECT_FALSE(Manager().GetMetadata(handle));
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

	// Asset::SetName prefixes with AssetTypeToString, which spells the enumerator out in
	// full — so the label is "AssetType::Scene - Level", not the "Scene - Level" the
	// doc comment on SetName suggests.
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
	// Unloading is not unregistering: the handle stays resolvable so the asset can be
	// reimported on the next use.
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

TEST_F(AssetRegistryTest, ResolvingAnAssetWhoseFileIsGoneReturnsNull)
{
	// Content deleted from disk while a scene still refers to it must resolve to null, not
	// throw: the handle stays registered, so this is reached simply by opening a project
	// whose files have moved.
	const std::filesystem::path relative = WriteScene("Scenes/Level.uge");
	const AssetHandle handle = Manager().ImportAsset(relative, "Level");
	Manager().SerializeAssetRegistry();

	// Reopen so the handle is registered but nothing is cached, then take the file away.
	ASSERT_TRUE(Project::Load(m_project.ProjectFile()));
	ASSERT_TRUE(std::filesystem::remove(Project::GetAssetFileSystemPath(relative)));

	Uge::Ref<Scene> resolved;
	EXPECT_NO_THROW({ resolved = AssetManager::GetAsset<Scene>(handle); });

	// Null rather than an empty scene: an importer that reported success here would let the
	// editor save a blank scene over content that was only misplaced.
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
