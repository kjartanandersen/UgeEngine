#pragma once



#ifdef UG_PLATFORM_WINDOWS

extern Uge::Application* Uge::CreateApplication();

int main(int argc, char** argv)
{
	

	Uge::Log::Init();

	UG_PROFILE_BEGIN_SESSION("Startup", "UgeProfile-Startup.json");
	auto app = Uge::CreateApplication();
	UG_PROFILE_END_SESSION();
	
	UG_PROFILE_BEGIN_SESSION("Runtime", "UgeProfile-Runtime.json");
	app->Run();
	UG_PROFILE_END_SESSION();
	
	UG_PROFILE_BEGIN_SESSION("Shutdown", "UgeProfile-Shutdown.json");
	delete app;
	UG_PROFILE_END_SESSION();

	return 0;
}

#endif // UG_PLATFORM_WINDOWS


