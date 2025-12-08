#pragma once



#ifdef UG_PLATFORM_WINDOWS

extern Uge::Application* Uge::CreateApplication();

int main(int argc, char** argv)
{
	auto app = Uge::CreateApplication();
	app->Run();

	delete app;

	return 0;
}

#endif // UG_PLATFORM_WINDOWS


