#pragma once

#include "Scene.h"

namespace Uge
{
	
	class SceneSerializer
	{

	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const std::filesystem::path& filepath);
		void SerializeRuntime(const std::filesystem::path& filepath);

		bool DeSerialize(const std::filesystem::path& filepath);
		bool DeSerializeRuntime(const std::filesystem::path& filepath);
	private:
		Ref<Scene> m_scene;

	};



}