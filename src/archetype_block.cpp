#include "include/archetype_block.hpp"

namespace peetcs
{
	archetype_block::archetype_block(const archetype_id& id):
		block(id.type_indices, 100000),
		archetype_info(id)
	{
	}

	storage::region archetype_block::add_entity(const entity_id entity)
	{
		auto region = block.add_element(entity);
		entity_index_map[entity] = region.index;
		last_entity_in_storage = entity;

		return region;
	}

	storage::region archetype_block::get_entity(const entity_id entity)
	{
		return block.get_element(entity_index_map[entity]);
	}

	void archetype_block::remove_entity(const entity_id entity)
	{
		const auto entity_to_remove_it = entity_index_map.find(entity);
		if (entity_to_remove_it == entity_index_map.end())
		{
			__debugbreak();
		}

		size_t          last_entity         = block.last();
		storage::region last_element_region = block.get_element(last_entity);
		block.remove_element(entity_to_remove_it->second);

		auto old_index = entity_to_remove_it->second;
		entity_index_map.erase(entity_to_remove_it->first);

		if (last_entity == old_index)
		{
			return;
		}

		entity_index_map[last_element_region.identifier] = old_index;

		/*for (auto& index_lookup : entity_index_map)
		{
			if (index_lookup.second == last_entity) [[unlikely]]
			{
				entity_index_map[index_lookup.first] = old_index;
			}
		}*/
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