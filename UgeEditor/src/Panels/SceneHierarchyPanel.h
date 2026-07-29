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
	 * @brief Lists the scene's entities and edits the components of the selected one.
	 * @ingroup group_editor
	 *
	 * Two panels in one: the hierarchy tree, and the properties inspector that draws an
	 * editing UI for each component on the selection.
	 *
	 * @note Every component type needs an editing UI added here, or it will be invisible in
	 * the editor even though it serializes correctly.
	 */
	class SceneHierarchyPanel
	{
	public:
		/** @brief Constructs the panel with no scene bound. */
		SceneHierarchyPanel() = default;
		/**
		 * @brief Constructs the panel bound to a scene.
		 * @param context Scene to display.
		 */
		SceneHierarchyPanel(const Ref<Scene>& context);

		/**
		 * @brief Binds the panel to a different scene and clears the selection.
		 * @param context Scene to display.
		 */
		void SetContext(const Ref<Scene>& context);

		/** @brief Draws the hierarchy tree and the properties inspector. */
		void OnImGuiRender();

		/**
		 * @brief The currently selected entity.
		 * @return The selection, or an invalid handle if nothing is selected.
		 */
		Entity GetSelectedEntity() const { return m_selectionContext; }
		/**
		 * @brief Sets the selection.
		 * @param entity Entity to select; an invalid handle clears the selection.
		 */
		void SetSelectedEntity(Entity entity);

	private:
		/**
		 * @brief Adds one entry to the "Add Component" menu, if the entity lacks that component.
		 * @tparam T Component type the entry adds.
		 * @param entryName Label shown in the menu.
		 */
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);

		/**
		 * @brief Draws one entity's row in the hierarchy tree.
		 * @param entity Entity to draw.
		 */
		void DrawEntityNode(Entity entity);
		/**
		 * @brief Draws the property editors for every component on an entity.
		 * @param entity Entity whose components to display.
		 */
		void DrawComponents(Entity entity);

	private:
		Ref<Scene> m_context;
		Entity m_selectionContext;

	};



}