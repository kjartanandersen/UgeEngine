#pragma once

#include "ugpch.h"

#include "Uge/Core.h"
#include "Uge/Events/Event.h"

namespace Uge
{

	struct WindowProps
	{

		std::string m_title;
		unsigned int m_width;
		unsigned int m_height;

		WindowProps( const std::string& title = "Uge Engine",
				unsigned int width = 1280, unsigned int height = 720) 
				: m_title(title), m_width(width), m_height(height)  {}
	};

	class UG_API Window
	{

	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;
		

		// Window Attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());



	};



}