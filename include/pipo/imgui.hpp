#pragma once

#include <algorithm>

#include "imgui.h"
#include "include/archetype_pool.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "include/utilities.hpp"

class gui_interface
{
public:
	virtual ~gui_interface() = default;
	virtual void draw(peetcs::archetype_pool& pool) = 0;
};



class performance_profiler final : public gui_interface
{
	char text_buffer[64];
	time_info time;

public:
	void draw(peetcs::archetype_pool& pool) override;
};

template<class T>
class dll_reloader final : public gui_interface
{
	static bool replace(std::string& str, const std::string& from, const std::string& to) {
		size_t start_pos = str.find(from);
		if (start_pos == std::string::npos)
			return false;
		str.replace(start_pos, from.length(), to);
		return true;
	}


public:
	void inline draw(peetcs::archetype_pool& pool) override
	{
		ImGui::Begin("Hot reload");


		std::string load_header = "Load DLL: ";
		std::string release_header = "Release DLL: ";
		std::string name = typeid(T).name();
		replace(name, "struct ", "");

		std::string load_text = load_header + name;
		std::string release_text = release_header + name;

		if (!initialised)
		{
			if (ImGui::Button(load_text.c_str()))
			{
				T::load_dll();
				initialised = true;
			}
		}
		else
		{
			T::tick(pool);

			if (ImGui::Button(release_text.c_str()))
			{
				T::release_dll();
				initialised = false;
			}
		}

		ImGui::End();
	}

private:
	bool initialised;

};