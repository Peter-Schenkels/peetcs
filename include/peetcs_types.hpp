#pragma once

namespace peetcs
{
	using entity_id = int;
	constexpr inline entity_id invalid_entity_id = std::numeric_limits<int>::max();



	struct type_info
	{
		std::type_index type_id;
		size_t size_of;
	};
}

