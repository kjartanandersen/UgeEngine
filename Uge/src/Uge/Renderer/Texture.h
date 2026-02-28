#pragma once

#include <string>

#include "Uge/Core/Core.h"

namespace Uge
{

	class Texture
	{

	public:
		virtual ~Texture() = default;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void UnBind(uint32_t slot = 0) const = 0;

		virtual bool operator==(const Texture& other) const = 0;

	};


	class Texture2D : public Texture
	{
		
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, const std::string& name);
		static Ref<Texture2D> Create(const std::string& path, const std::string& name = "");

	public:
		std::string m_name;
		std::string m_path;
		uint32_t m_width;
		uint32_t m_height;



	protected:


	};

}


