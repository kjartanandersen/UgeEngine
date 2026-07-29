/**
 * @file UgeEditorApp.cpp
 * @brief Entry point of the Uge-Editor application.
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
	 * @brief The editor client application.
	 * @ingroup group_editor
	 *
	 * Pushes Uge::EditorLayer, which holds all of the editor's behaviour.
	 * @see Uge::CreateApplication
	 */
	class UgeEditor : public Uge::Application
	{
	public:
		/**
		 * @brief Constructs the application and pushes the editor layer.
		 * @param spec Startup configuration from Uge::CreateApplication.
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

