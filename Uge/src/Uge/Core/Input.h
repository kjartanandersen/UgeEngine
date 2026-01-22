#pragma once

#include "Uge/Core/Core.h"

namespace Uge
{

	class Input
	{

	public:
		
		static bool IsKeyPressed(int keyCode);
		static bool IsMouseButtonPressed(int button);

		static float GetMouseX();
		static float GetMouseY();
		static std::pair<float, float> GetMousePos();

	};




}


