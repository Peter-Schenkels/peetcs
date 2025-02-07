#include "include/query_value.hpp"
#include "include/generic_container.hpp"

namespace peetcs
{
	query_value::query_value(element_layout& memory_accesor, const generic_container::element_rep& region) :
		memory_accesor(memory_accesor),
		region(region)
	{
	}
}
