#pragma once



#ifdef UG_PLATFORM_WINDOWS

extern Uge::Application* Uge::CreateApplication();

int main(int argc, char** argv)
{
	int a = 42;

	Uge::Log::Init();
	UG_CORE_WARN("Initialized Log!");
	UG_INFO("Hello Var={0}!", a);

	auto app = Uge::CreateApplication();
	app->Run();

	delete app;

	return 0;
}

#endif // UG_PLATFORM_WINDOWS


