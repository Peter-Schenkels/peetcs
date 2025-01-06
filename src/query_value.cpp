#include "include/query_value.hpp"

namespace peetcs
{
	query_value::query_value(archetype_id& memory_accesor, const storage::region& region) :
		memory_accesor(memory_accesor),
		region(region)
	{
	}
}