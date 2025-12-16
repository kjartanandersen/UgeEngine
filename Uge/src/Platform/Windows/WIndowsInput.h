#pragma once

#include "Uge/Input.h"

namespace Uge
{

	class WindowsInput : public Input
	{

	protected:
		virtual bool IsKeyPressedImpl(int keyCode);
		virtual bool IsMouseButtonPressedImpl(int button) override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
		virtual std::pair<float, float> GetMousePosImpl() override;



	};




}

