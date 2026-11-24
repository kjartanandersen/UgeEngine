/**
 * @file SceneSerializer.h
 * @brief Reads and writes scenes as YAML `.uge` files.
 * @ingroup group_scene
 */

#pragma once

#include "Scene.h"

namespace Uge
{
	
	/**
	 * @brief Reads and writes a scene's entities and components as YAML.
	 * @ingroup group_scene
	 *
	 * A component type it doesn't handle is lost on save.
	 */
	class SceneSerializer
	{

	public:
		/**
		 * @brief Creates a serializer for a scene.
		 * @param scene Scene to read into or write from.
		 */
		SceneSerializer(const Ref<Scene>& scene);

		/**
		 * @brief Writes the scene.
		 * @param filepath The `.uge` file; replaced if it exists.
		 * @return `true` on success. On failure the reason is logged and the file is unchanged.
		 *
		 * Writes `<filepath>.tmp`, then renames it over the file.
		 */
		bool Serialize(const std::filesystem::path& filepath);
		/**
		 * @brief Writes the scene in a packed form. Not implemented; asserts.
		 * @param filepath The file.
		 */
		void SerializeRuntime(const std::filesystem::path& filepath);

		/**
		 * @brief Reads a scene, adding its entities to the scene.
		 * @param filepath The `.uge` file.
		 * @return `false` if the file is missing or malformed.
		 */
		bool DeSerialize(const std::filesystem::path& filepath);
		/**
		 * @brief Reads a packed scene. Not implemented; asserts.
		 * @param filepath The file.
		 * @return `true` on success.
		 */
		bool DeSerializeRuntime(const std::filesystem::path& filepath);
	private:
		Ref<Scene> m_scene;

	};



}