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
	 * @brief Serializes a scene's entities and components to YAML, and back.
	 * @ingroup group_scene
	 *
	 * Each entity is written with its UUID, so references between entities survive a
	 * round-trip. Assets are stored as handles, never as paths, which means a file can be
	 * moved on disk without breaking the scene that uses it.
	 *
	 * @code
	 * SceneSerializer serializer(scene);
	 * serializer.Serialize("assets/scenes/Level1.uge");
	 * @endcode
	 *
	 * @warning Every component type needs explicit read and write support here. A new
	 * component that is not handled is silently dropped on save.
	 */
	class SceneSerializer
	{

	public:
		/**
		 * @brief Binds the serializer to a scene.
		 * @param scene Scene to read from or write into.
		 */
		SceneSerializer(const Ref<Scene>& scene);

		/**
		 * @brief Writes the scene as human-readable YAML.
		 * @param filepath Destination `.uge` path; overwritten if it exists.
		 */
		void Serialize(const std::filesystem::path& filepath);
		/**
		 * @brief Writes the scene in a packed runtime form.
		 * @param filepath Destination path.
		 * @note Not implemented yet; asserts if called.
		 */
		void SerializeRuntime(const std::filesystem::path& filepath);

		/**
		 * @brief Loads a YAML scene into the bound scene.
		 * @param filepath Source `.uge` path.
		 * @return `true` on success; `false` if the file is missing or malformed.
		 * @note Entities are added to the bound scene rather than replacing its contents.
		 */
		bool DeSerialize(const std::filesystem::path& filepath);
		/**
		 * @brief Loads a packed runtime scene.
		 * @param filepath Source path.
		 * @return `true` on success.
		 * @note Not implemented yet; asserts if called.
		 */
		bool DeSerializeRuntime(const std::filesystem::path& filepath);
	private:
		Ref<Scene> m_scene;

	};



}