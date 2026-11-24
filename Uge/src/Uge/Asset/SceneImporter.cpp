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

		// A failed load returns null, so a broken file can't be opened as an empty scene and
		// saved over.
		if (!serializer.DeSerialize(path))
		{
			UG_CORE_ERROR("SceneImporter::LoadScene - Failed to load scene '{0}'", path.string());
			return nullptr;
		}

		return scene;


	}
	bool SceneImporter::SaveScene(Ref<Scene> scene, const std::filesystem::path& path)
	{


		UG_PROFILE_FUNCTION();


		SceneSerializer serializer(scene);
		return serializer.Serialize(Project::GetAssetDirectory() / path);


	}
}