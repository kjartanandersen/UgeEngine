#include <ugpch.h>
#include "SceneImporter.h"

#include "Uge/Project/Project.h"

#include "Uge/Scene/SceneSerializer.h"


namespace Uge
{
	Ref<Scene> SceneImporter::ImportScene(AssetHandle handle, const AssetMetadata& metadata)
	{

		UG_PROFILE_FUNCTION();

		return LoadScene(Project::GetAssetDirectory() / metadata.FilePath);

		
	}

	Ref<Scene> SceneImporter::LoadScene(const std::filesystem::path& path)
	{

		UG_PROFILE_FUNCTION();

		Ref<Scene> scene = CreateRef<Scene>();
		SceneSerializer serializer(scene);

		// Honour the result rather than handing back the half-built scene. A missing or
		// malformed file would otherwise import as an empty scene that looks loaded — and
		// saving over the original from there loses it for good.
		if (!serializer.DeSerialize(path))
		{
			UG_CORE_ERROR("SceneImporter::LoadScene - Failed to load scene '{0}'", path.string());
			return nullptr;
		}

		return scene;


	}
	void SceneImporter::SaveScene(Ref<Scene> scene, const std::filesystem::path& path)
	{


		UG_PROFILE_FUNCTION();


		SceneSerializer serializer(scene);
		serializer.Serialize(Project::GetAssetDirectory() / path);


	}
}