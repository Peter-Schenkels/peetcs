#pragma once

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