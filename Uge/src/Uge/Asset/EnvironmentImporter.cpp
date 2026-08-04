#include <ugpch.h>

#include "EnvironmentImporter.h"

#include "Uge/Project/Project.h"

namespace Uge
{
	Ref<Environment> EnvironmentImporter::ImportEnvironment(AssetHandle handle, const AssetMetadata& metadata)
	{
		UG_PROFILE_FUNCTION();

		Ref<Environment> environment = Environment::Create(Project::GetAssetDirectory() / metadata.FilePath);
		if (environment)
		{
			environment->SetName(metadata.FilePath.string());
			UG_CORE_INFO("EnvironmentImporter - built environment from {0}", metadata.FilePath.string());
		}

		return environment;
	}
}
