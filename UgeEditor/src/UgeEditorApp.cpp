#include <Uge.h>
// ************* Entry Point **************
#include "Uge/Core/EntryPoint.h"
// ****************************************

#include "EditorLayer.h"



class UgeEditor : public Uge::Application
{
public:
	UgeEditor()
		: Application(false, "Uge Editor")
	{

		PushLayer( new Uge::EditorLayer());
	}
	~UgeEditor()
	{

	}

private:
	
};

Uge::Application* Uge::CreateApplication()
{
	return new UgeEditor();
}