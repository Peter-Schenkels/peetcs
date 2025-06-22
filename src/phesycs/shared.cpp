#include "tests/shared.hpp"

#include <iostream>

#include "include/archetype_pool.hpp"
#include "include/phesycs/phesycs.hpp"
#include "include/pipo/rasterizer.hpp"


void tick_api(peetcs::archetype_pool& c_pool, pipo& gpu_context)
{
	peetcs::archetype_pool& pool = c_pool;
	auto query = pool.query<pipo::transform_data, pipo::mesh_renderer_data, pipo::unlit_material_data>();

	int i = 0;
	for (auto query_value : query)
	{
		pipo::transform_data& transform_data = query_value.get<pipo::transform_data>();
		transform_data.position[0] += 0.01f / float(i);


		glm::vec3 dir = glm::normalize(transform_data.get_pos() * 1.f);
		transform_data.set_pos(transform_data.get_pos() - dir * 0.001f);

		i++;
	}

	phesycs_impl::tick(pool, gpu_context);
}



