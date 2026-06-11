#include <ugpch.h>

#include "MeshImporter.h"


#include "Uge/Project/Project.h"

namespace Uge
{
	Ref<Model> MeshImporter::ImportMesh(AssetHandle handle, const AssetMetadata& metadata)
	{

		UG_PROFILE_FUNCTION();

		auto model = CreateRef<Model>((Project::GetAssetDirectory() / metadata.FilePath).string());
		model->m_handle = handle;
		return model;

	}

	
}