#include "include/archetype_pool.hpp"
#include <ostream>

namespace peetcs
{
	void archetype_pool::emplace_commands()
	{
		int i = 0;
		for (const auto& add_command : add_commands)
		{
			execute_add(add_command.target, add_command.component_data, add_command.component_type, add_command.component_size);
			free(add_command.component_data);
			i++;
		}/**/

		for (const auto& remove_command : remove_commands)
		{
			execute_remove(remove_command);
		}

		add_commands.clear();
		remove_commands.clear();
	}

	generic_container::element_rep archetype_pool::execute_add(entity_id entity, void* data, element_layout::hash_t component_type,
		std::size_t component_size)
	{
		// Scenario 1 (Entity does not exist)
		auto archetype_it = entity_archetype_lookup.find(entity);
		if (archetype_it == entity_archetype_lookup.end()) [[likely]]
		{
			element_layout memory_descriptor = {};
			memory_descriptor.add(component_type, component_size);

			const auto block_it = blocks.find(memory_descriptor);

			// Scenario 2 (Block doesnt exist and entity doesn't exist)
			if (block_it == blocks.end())
			{
				blocks[memory_descriptor] = generic_container(memory_descriptor, static_cast<element_layout::index_t>(default_array_size));
			}

			generic_container::element_rep region = blocks[memory_descriptor].add_element(entity);
			region.set_generic_ptr(component_type, component_size, data);

			entity_archetype_lookup[entity] = memory_descriptor;

			return region;
		}


		// Scenario 3 (Entity exist already, move to a new archetype
		{
			element_layout memory_descriptor_old = archetype_it->second;
			const auto old_block_it = blocks.find(memory_descriptor_old);

			element_layout memory_descriptor_new = memory_descriptor_old;
			memory_descriptor_new.add(component_type, component_size);
			auto new_block_it = blocks.find(memory_descriptor_new);

			// Scenario 4 Block does not exist for new memory descriptor
			if (new_block_it == blocks.end()) [[unlikely]]
			{
				if (old_block_it == blocks.end()) [[unlikely]]
				{
					__debugbreak();
				}

				blocks[memory_descriptor_new] = generic_container { memory_descriptor_new, static_cast<element_layout::index_t>(default_array_size) };
				new_block_it = blocks.find(memory_descriptor_new);
			}

			// Scenario 5 Block exists and needs to be migrated
			generic_container::element_rep new_region = new_block_it->second.add_element(entity);
			generic_container::element_rep old_region = old_block_it->second.get_element(entity);

			new_region.set_generic_ptr(component_type, component_size, data);
			new_region.copy_sub_elements(old_region);

			entity_archetype_lookup[entity] = memory_descriptor_new;

			old_block_it->second.remove(entity);

			return new_region;
		}
	}

	void archetype_pool::execute_remove(const remove_component_command& command)
	{
		element_layout& old_archetype_id = entity_archetype_lookup[command.target];
		element_layout new_archetype_id = old_archetype_id;
		new_archetype_id.remove(command.component_type);

		if (!new_archetype_id.layout.empty()) [[likely]]
		{
			generic_container::element_rep old_region = blocks[old_archetype_id].get_element(command.target);
			if (old_region.get_id() == std::numeric_limits<element_layout::id_t>::max()) [[unlikely]]
			{
				// element does not exist
				return;
			}

			if (!blocks.contains(new_archetype_id)) [[unlikely]]
			{
				blocks[new_archetype_id] = generic_container{ new_archetype_id, static_cast<element_layout::index_t>(default_array_size) };
			}

			generic_container::element_rep new_region = blocks[new_archetype_id].add_element(command.target);
			new_region.copy_sub_elements(old_region);

			blocks[old_archetype_id].remove(command.target);
			entity_archetype_lookup[command.target] = new_archetype_id;
		}
		else
		{
			blocks[old_archetype_id].remove(command.target);
			entity_archetype_lookup.erase(command.target);
		}
	}
}