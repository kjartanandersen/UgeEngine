#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Uge/Renderer/Model.h"


namespace Uge
{

	class MeshImporter
	{

	public:
		static Ref<Model> ImportMesh(AssetHandle handle, const AssetMetadata& metadata);



	};

}