#include "include/archetype_block.hpp"

namespace peetcs
{
	archetype_block::archetype_block(const archetype_id& id):
		block(id.type_indices, 1000),
		archetype_info(id)
	{
	}

	storage::region archetype_block::add_entity(const entity_id entity)
	{
		auto region = block.add_element();
		entity_index_map[entity] = region.index;

		return region;
	}

	storage::region archetype_block::get_entity(const entity_id entity)
	{
		return block.get_element(entity_index_map[entity]);
	}

	void archetype_block::remove_entity(const entity_id entity)
	{
		const auto entity_it = entity_index_map.find(entity);
		if (entity_it == entity_index_map.end())
		{
			__debugbreak();
		}

		size_t last_entity = block.last();
		block.remove_element(entity_it->second);
		entity_index_map.erase(entity_it->first);

		bool is_last = true;
		for (auto& element : entity_index_map)
		{
			// Move last to deleted entity index
			if (element.second == last_entity)
			{
				entity_index_map[element.first] = entity_it->second;
				is_last = false;
			}
		}
	}

	archetype_block::iterator::iterator(const storage::iterator& it):
		it(it)
	{
	}

	archetype_block::iterator& archetype_block::iterator::operator++()
	{
		++it;
		return *this;
	}

	storage::iterator archetype_block::iterator::operator*() const
	{
		return it;
	}

	storage::iterator archetype_block::iterator::operator->() const
	{
		return it;
	}

	bool archetype_block::iterator::operator==(const iterator& other) const
	{
		return it == other.it;
	}

	bool archetype_block::iterator::operator!=(const iterator& other) const
	{
		return it != other.it;
	}

	archetype_block::iterator archetype_block::begin()
	{
		return iterator{ block.begin() };
	}

	archetype_block::iterator archetype_block::end()
	{
		return iterator{ block.end() };
	}
}