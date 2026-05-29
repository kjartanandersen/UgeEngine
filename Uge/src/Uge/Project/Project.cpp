#include <ugpch.h>
#include "Project.h"

#include "ProjectSerializer.h"

namespace Uge
{



	Ref<Project> Project::New()
	{

		s_activeProject = CreateRef<Project>();
		return s_activeProject;

	}

	Ref<Project> Project::Load(const std::filesystem::path& path)
	{
		Ref<Project> project = CreateRef<Project>();

		ProjectSerializer serializer(project);

		if (serializer.Deserialize(path))
		{
			project->m_ProjectDirectory = path.parent_path();
			s_activeProject = project;
			std::shared_ptr<EditorAssetManager> editorAssetManager = std::make_shared<EditorAssetManager>();
			s_activeProject->m_assetManager = editorAssetManager;
			editorAssetManager->DeserializeAssetRegistry();
			return s_activeProject;
		}

		return nullptr;

	}

	bool Project::SaveActive(const std::filesystem::path& path)
	{
		ProjectSerializer serializer(s_activeProject);

		if (serializer.Serialize(path))
		{
			s_activeProject->m_ProjectDirectory = path.parent_path();
			return true;

		}
		return false;
	}

}