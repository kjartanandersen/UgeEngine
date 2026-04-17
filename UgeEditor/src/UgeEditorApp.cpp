#include <Uge.h>

// ************* Entry Point **************
#include "Uge/Core/EntryPoint.h"
// ****************************************

#include "EditorLayer.h"

namespace Uge
{

	class UgeEditor : public Uge::Application
	{
	public:
		UgeEditor(const ApplicationSpecification& spec)
			: Application(spec)
		{

			PushLayer(new Uge::EditorLayer());
		}
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

