#ifndef SHARED_LIBRARY_H
#define SHARED_LIBRARY_H

#include "include/archetype_pool.hpp"
#include "include/dll_macros.hpp"

API_DECLARE_FUNC(void, tick, peetcs::archetype_pool&)

struct phesycs
{
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
	}

	static void tick(peetcs::archetype_pool& pool)
	{
		tick_ptr(pool);
	}
};



#endif


