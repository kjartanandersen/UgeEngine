#include "Application.h"



#include "Uge/Events/ApplicationEvent.h"
#include "Uge/Log.h"

namespace Uge
{
	Application::Application()
	{

	}

	Application::~Application()
	{

	}

	void Application::Run()
	{

		WindowResizeEvent e(1280, 720);

		if (e.IsInCategory(EventCategoryApplication))
		{
			UG_TRACE(e.ToString());

		}


		if (e.IsInCategory(EventCategoryInput))
		{
			UG_TRACE(e.ToString());

		}

		while (true)
		{

		}
	}

}

