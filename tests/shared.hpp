#ifndef SHARED_LIBRARY_H
#define SHARED_LIBRARY_H

#include "include/archetype_pool.hpp"
#include "include/dll_macros.hpp"
#include "include/pipo/rasterizer.hpp"

typedef void (*tick_func_ptr_t)(peetcs::archetype_pool&, pipo&); inline tick_func_ptr_t tick_ptr;

extern "C" {
	__declspec(dllimport) void tick_api(peetcs::archetype_pool&, pipo&);
}

struct phesycs
{
	inline static bool loaded = false;
	inline static HMODULE dll;

	static int load_dll()
	{
		API_LOAD_DLL(PHESYCS)
		API_DEFINE_FUNC(tick)

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
};



#endif


