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
		UgeEditor(Uge::ApplicationCommandLineArgs args)
			: Application(false, "Uge Editor", args)
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
		return new UgeEditor(args);
	}


}

