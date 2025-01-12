#include "include/archetype_pool.hpp"

#include "include/query_value.hpp"

namespace peetcs
{
	void archetype_pool::emplace_commands()
	{
		for (const auto& add_command : add_commands)
		{
			execute_add(add_command.target, add_command.component_data, add_command.component_type, add_command.component_size);
			free(add_command.component_data);
		}

		for (const auto& remove_command : remove_commands)
		{
			execute_remove(remove_command);
		}

		add_commands.clear();
		remove_commands.clear();
	}

	void archetype_pool::execute_add(entity_id entity, void* data, std::type_index component_type,
		std::size_t component_size)
	{
		// Scenario 1 (Entity does not exist)
		auto archetype_it = entity_archetype_lookup.find(entity);
		if (archetype_it == entity_archetype_lookup.end())
		{
			archetype_id memory_descriptor = {};
			memory_descriptor.init(component_type, component_size);

			const auto block_it = blocks.find(memory_descriptor);

			// Scenario 2 (Block doesnt exist and entity doesn't exist)
			if (block_it == blocks.end())
			{
				blocks[memory_descriptor] = archetype_block(memory_descriptor);
			}

			const storage::region region = blocks[memory_descriptor].add_entity(entity);
			memory_descriptor.set_component_ptr(component_type, data, region);
			entity_archetype_lookup[entity] = memory_descriptor;

			return;
		}


		// Scenario 3 (Entity exist already, move to a new archetype
		{
			archetype_id memory_descriptor_old = archetype_it->second;
			const auto old_block_it = blocks.find(memory_descriptor_old);

			archetype_id memory_descriptor_new = memory_descriptor_old;
			memory_descriptor_new.add_type(component_type, component_size);
			auto new_block_it = blocks.find(memory_descriptor_new);

			// Scenario 4 Block does not exist for new memory descriptor
			if (new_block_it == blocks.end())
			{
				if (old_block_it == blocks.end())
				{
					__debugbreak();
				}

				blocks[memory_descriptor_new] = archetype_block{ memory_descriptor_new };
				new_block_it = blocks.find(memory_descriptor_new);
			}

			// Scenario 5 Block exists and needs to be migrated
			storage::region old_region = old_block_it->second.get_entity(entity);
			storage::region new_region = new_block_it->second.add_entity(entity);
			memory_descriptor_new.set_component_ptr(component_type, data, new_region);
			memory_descriptor_new.migrate(new_region, old_region, memory_descriptor_old);

			entity_archetype_lookup[entity] = memory_descriptor_new;

			old_block_it->second.remove_entity(entity);
		}
	}

	void archetype_pool::execute_remove(const remove_component_command& command)
	{
		archetype_id& old_archetype_id = entity_archetype_lookup[command.target];
		archetype_id new_archetype_id = old_archetype_id;
		new_archetype_id.remove_type(command.component_type);

		if (new_archetype_id.get_size_of_element() > 0)
		{
			storage::region old_region = blocks[old_archetype_id].get_entity(command.target);

			if (!blocks.contains(new_archetype_id))
			{
				blocks[new_archetype_id] = archetype_block{ new_archetype_id };
			}

			storage::region new_region = blocks[new_archetype_id].add_entity(command.target);
			new_archetype_id.migrate(new_region, old_region, old_archetype_id);

			blocks[old_archetype_id].remove_entity(command.target);
			entity_archetype_lookup[command.target] = new_archetype_id;
		}
		else
		{
			blocks[old_archetype_id].remove_entity(command.target);
			entity_archetype_lookup.erase(command.target);
		}
	}
}