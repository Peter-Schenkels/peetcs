#include "tests/shared.hpp"

#include <iostream>

#include "include/archetype_pool.hpp"
#include "include/pipo/rasterizer.hpp"


void tick_api(peetcs::archetype_pool& c_pool)
{
	peetcs::archetype_pool& pool = c_pool;
	auto query = pool.query<pipo::transform_data, pipo::mesh_renderer_data, pipo::unlit_material_data>();

	for (auto query_value : query)
	{
		pipo::transform_data& transform_data = query_value.get<pipo::transform_data>();
		transform_data.position[0] += 0.01f;

	}
}