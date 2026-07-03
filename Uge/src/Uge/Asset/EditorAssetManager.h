#pragma once

#include "AssetManagerBase.h"
#include "AssetMetadata.h"
#include <map>

namespace Uge
{
	using AssetRegistry = std::map<AssetHandle, AssetMetadata>;
	using MeshAssetRegistry = std::map<AssetHandle, MeshAssetMetadata>;

	class EditorAssetManager : public AssetManagerBase
	{

	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) override;
		
		virtual bool IsAssetHandleValid(AssetHandle handle) const override;
		virtual bool IsMeshAssetHandleValid(AssetHandle handle) const override;
		virtual bool IsAssetLoaded(AssetHandle handle) const override;
		virtual AssetType GetAssetType(AssetHandle handle) const override;


		AssetHandle ImportAsset(const std::filesystem::path& filepath, const std::string& name = "");
		AssetHandle GetOrImportAsset(const std::filesystem::path& filepath, const std::string& name = "");

		// Registers an asset that has no backing file (e.g. materials generated during
		// mesh import). Assigns and returns a new handle. Memory-only assets are not
		// serialized to the asset registry.
		AssetHandle AddMemoryOnlyAsset(const Ref<Asset>& asset);


		const AssetMetadata& GetMetadata(AssetHandle handle) const;
		const std::filesystem::path& GetFilePath(AssetHandle handle) const;
		const MeshAssetMetadata& GetMeshMetadata(AssetHandle handle) const;
		void SetMeshMetadata(AssetHandle handle, const MeshAssetMetadata& metadata);
		std::vector<std::string> GetLoadedAssetsNames();

		const AssetRegistry& GetAssetRegistry() const { return m_assetRegistry; }


		void SerializeAssetRegistry();
		bool DeserializeAssetRegistry();

	private:
		AssetRegistry m_assetRegistry;
		MeshAssetRegistry m_meshAssetRegistry;
		AssetMap  m_loadedAssets;

		// Assets with no backing file (e.g. materials generated during mesh import).

	};

}