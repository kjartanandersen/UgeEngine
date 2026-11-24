/**
 * @file SceneHierarchyPanel.h
 * @brief The entity list and the property inspector.
 * @ingroup group_editor
 */

#pragma once

#include <Uge.h>

namespace Uge
{
	/**
	 * @brief The entity tree and the properties of the selected entity's components.
	 * @ingroup group_editor
	 *
	 * A component without UI here can't be seen or edited in the editor.
	 */
	class SceneHierarchyPanel
	{
	public:
		/** @brief A panel with no scene. */
		SceneHierarchyPanel() = default;
		/**
		 * @brief A panel showing a scene.
		 * @param context The scene.
		 */
		SceneHierarchyPanel(const Ref<Scene>& context);

		/**
		 * @brief Shows another scene and clears the selection.
		 * @param context The scene.
		 */
		void SetContext(const Ref<Scene>& context);

		/** @brief Draws the entity tree and the properties panel. */
		void OnImGuiRender();

		/**
		 * @brief The selected entity.
		 * @return The entity, or a null entity.
		 */
		Entity GetSelectedEntity() const { return m_selectionContext; }
		/**
		 * @brief Selects an entity.
		 * @param entity The entity; a null entity clears the selection.
		 */
		void SetSelectedEntity(Entity entity);

		/**
		 * @brief Sets the font of the X, Y and Z buttons on vector fields, for every panel.
		 * @param font A font in ImGui's atlas, or `nullptr` for the current font.
		 */
		static void SetBoldFont(ImFont* font);

	private:
		/**
		 * @brief Adds an "Add Component" menu entry, if the entity has no `T`.
		 * @tparam T The component type.
		 * @param entryName The entry's label.
		 */
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);

		/**
		 * @brief Draws an entity's row in the tree.
		 * @param entity The entity.
		 */
		void DrawEntityNode(Entity entity);
		/**
		 * @brief Draws the properties of an entity's components.
		 * @param entity The entity.
		 */
		void DrawComponents(Entity entity);

	private:
		Ref<Scene> m_context;
		Entity m_selectionContext;

	};



}