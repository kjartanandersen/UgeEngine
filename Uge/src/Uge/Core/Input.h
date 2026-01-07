#pragma once

#include "Uge/Core/Core.h"

namespace Uge
{

	class Input
	{

	public:
		inline static bool IsKeyPressed(int keyCode) { return s_instance->IsKeyPressedImpl(keyCode); }
		inline static bool IsMouseButtonPressed(int button) { return s_instance->IsMouseButtonPressedImpl(button); }
		inline static float GetMouseX() { return s_instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_instance->GetMouseYImpl(); }
		inline static std::pair<float, float> GetMousePos() { return s_instance->GetMousePosImpl(); }


	protected:
		virtual bool IsKeyPressedImpl(int keyCode) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;
		virtual std::pair<float, float> GetMousePosImpl() = 0;


	private:
		static Input* s_instance;

	};




}


