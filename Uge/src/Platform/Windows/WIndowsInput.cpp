
#include <ugpch.h>
#include "Uge/Core/Input.h"

#include "Uge/Core/Application.h"
#include <GLFW/glfw3.h>


namespace Uge
{

	bool Input::IsKeyPressed(int keyCode)
	{

		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		auto state = glfwGetKey(window, keyCode);

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());

		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePos();

		return x;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePos();

		return y;
	}

	std::pair<float, float> Input::GetMousePos()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

}
