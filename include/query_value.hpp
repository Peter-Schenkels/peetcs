#pragma once
#include "archetype_id.hpp"

namespace peetcs
{
	struct query_value
	{
		archetype_id& memory_accesor;
		storage::region region;

		query_value(archetype_id& memory_accesor, const storage::region& region);

		template<typename Component>
		Component& get()
		{
			return memory_accesor.get_component<Component>(region);
		}
	};
}