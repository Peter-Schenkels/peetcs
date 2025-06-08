#include "include/pipo/imgui.hpp"

void performance_profiler::draw(peetcs::archetype_pool& pool)
{
	time.tick();

	memset(text_buffer, 0, sizeof text_buffer);


	double fps = 1.f / time.delta_time;

	ImGui::Begin("Performance Profiler");
	sprintf(text_buffer, "Frame time: %0.3fms \nFPS: %0.1f", time.delta_time, fps);
	ImGui::Text(text_buffer);
	ImGui::End();
}
