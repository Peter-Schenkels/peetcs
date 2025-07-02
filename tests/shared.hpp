#ifndef SHARED_LIBRARY_H
#define SHARED_LIBRARY_H

#include "include/archetype_pool.hpp"
#include "include/dll_macros.hpp"
#include "include/phesycs/phesycs.hpp"
#include "include/pipo/rasterizer.hpp"

DECLARE_API_FUNC(tick, void, peetcs::archetype_pool&, pipo&)
DECLARE_API_FUNC(apply_linear_impulse, void, phesycs_impl::rigid_body_data&, const glm::vec3&)
DECLARE_API_FUNC(apply_angular_impulse, void, phesycs_impl::rigid_body_data&, const glm::vec3&, const glm::vec3&)

struct phesycs
{
	inline static bool loaded = false;
	inline static HMODULE dll;

	static int load_dll()
	{
		API_LOAD_DLL(PHESYCS)
		API_DEFINE_FUNC(tick)
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
		tick_ptr(pool, gpu_context);
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


