#ifndef SHARED_LIBRARY_H
#define SHARED_LIBRARY_H

#include "include/archetype_pool.hpp"
#include "include/dll_macros.hpp"
#include "include/phesycs/phesycs.hpp"
#include "include/pipo/rasterizer.hpp"

DECLARE_API_FUNC(tick_integration, void, peetcs::archetype_pool&, pipo&)
DECLARE_API_FUNC(tick_collision_response, void, peetcs::archetype_pool&, pipo&)
DECLARE_API_FUNC(apply_linear_impulse, void, phesycs_impl::rigid_body_data&, const glm::vec3&)
DECLARE_API_FUNC(apply_angular_impulse, void, phesycs_impl::rigid_body_data&, const glm::vec3&, const glm::vec3&)

struct phesycs
{
	inline static bool loaded = false;
	inline static HMODULE dll;

	inline static float time_since_last_tick;
	inline static double system_time;
	inline static time_info current_time;

	static int load_dll()
	{
		API_LOAD_DLL(PHESYCS)
		API_DEFINE_FUNC(tick_integration)
		API_DEFINE_FUNC(tick_collision_response)
		API_DEFINE_FUNC(apply_linear_impulse)
		API_DEFINE_FUNC(apply_angular_impulse)

		return 0;
	}

	static void release_dll()
	{
		FreeLibrary(dll);
		loaded = false;
	}

	static void tick(peetcs::archetype_pool& pool, pipo& gpu_context)
	{
		current_time.tick();

		tick_integration_ptr(pool, gpu_context);

		if (current_time.get_now() - time_since_last_tick > 1.f / 60.f)
		{
			tick_collision_response_ptr(pool, gpu_context);
			current_time.tick();
			system_time = current_time.get_delta_time();
			time_since_last_tick = current_time.get_now();
		}

	}

	// Apply a linear impulse directly to the center of mass
	static void apply_linear_impulse(phesycs_impl::rigid_body_data& body, const glm::vec3& impulse)
	{
		apply_linear_impulse_ptr(body, impulse);
	}

	// Apply an angular impulse using the cross product of the contact vector and impulse
	static void apply_angular_impulse(phesycs_impl::rigid_body_data& body, const glm::vec3& impulse, const glm::vec3& contact_vector)
	{
		apply_angular_impulse_ptr(body, impulse, contact_vector);
	}
};



#endif


