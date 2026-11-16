/**
 * @file UgeEditorApp.cpp
 * @brief The editor application.
 * @ingroup group_editor
 */

#include <Uge.h>

// ************* Entry Point **************
#include "Uge/Core/EntryPoint.h"
// ****************************************

#include "EditorLayer.h"

namespace Uge
{

	/**
	 * @brief The editor application, which runs Uge::EditorLayer.
	 * @ingroup group_editor
	 */
	class UgeEditor : public Uge::Application
	{
	public:
		/**
		 * @brief Creates the application and pushes the editor layer.
		 * @param spec The startup settings.
		 */
		UgeEditor(const ApplicationSpecification& spec)
			: Application(spec)
		{

			PushLayer(new Uge::EditorLayer());
		}
		/** @brief Destroys the application. */
		~UgeEditor()
		{

		}

	private:

	};

	Uge::Application* Uge::CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "UgeEditor";
		spec.CommandLineArgs = args;


		return new UgeEditor(spec);
	}


}

