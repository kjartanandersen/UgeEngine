#include "ugpch.h"
#include "ImGuiLayer.h"


#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Uge/Core/Application.h"

// TODO: TEMPORARY
#include <GLFW/glfw3.h>
#include <glad/glad.h>


namespace Uge
{





	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{



	}

	ImGuiLayer::~ImGuiLayer()
	{




	}

	



	void ImGuiLayer::OnAttach()
	{
		UG_PROFILE_FUNCTION();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{

			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;

		}

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());


		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 410");
        
		io.Fonts->AddFontDefault();
		mainFont = io.Fonts->AddFontFromFileTTF("C:\\Programming\\c++\\GameEngines\\Uge\\Uge\\assets\\fonts\\PlayfairDisplayBold-nRv8g.ttf", 32.5f);
		IM_ASSERT(mainFont != NULL);




	}
	void ImGuiLayer::OnDetach()
	{
		UG_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();


	}

	void ImGuiLayer::OnImGuiRender()
	{

		


	}

	void Uge::ImGuiLayer::Begin()
	{
		UG_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


	}

	void Uge::ImGuiLayer::End()
	{
		UG_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();

		Application& app = Application::Get();

		io.DisplaySize = ImVec2(static_cast<float>(app.GetWindow().GetWidth()), 
								static_cast<float>(app.GetWindow().GetHeight()));

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{

			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);


		}

	}

}