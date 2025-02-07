#pragma once
#include "generic_container.hpp"

namespace peetcs
{
	struct query_value
	{
		element_layout& memory_accesor;
		generic_container::element_rep region;

		query_value(element_layout& memory_accesor, const generic_container::element_rep& region);

		template<typename Component>
		Component& get()
		{
			return region.get<Component>();
		}
	};
}
