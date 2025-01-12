#pragma once

#include <map>

#include "archetype_block.hpp"
#include "archetype_id.hpp"
#include "query_value.hpp"

#include <ranges>

namespace peetcs
{
	struct add_component_command
	{
		entity_id target;
		std::type_index component_type;
		std::size_t component_size;
		void* component_data;
	};

	struct remove_component_command
	{
		entity_id target;
		std::type_index component_type;
	};

	struct archetype_pool
	{
		std::unordered_map<archetype_id, archetype_block, archetype_hash> blocks;
		std::unordered_map<entity_id, archetype_id>                       entity_archetype_lookup;
		std::vector<add_component_command>                                add_commands;
		std::vector<remove_component_command>                             remove_commands;

		archetype_block empty_block = {};

		archetype_pool()
		{
			add_commands.reserve(100000);
		}


		template<typename Component>
		Component& add(const entity_id entity)
		{
			add_component_command command = {
				.target= entity,
				.component_type= typeid(Component),
				.component_size= sizeof(Component),
				.component_data= malloc(sizeof(Component))
			};

			*static_cast<Component*>(command.component_data) = Component{};

			add_commands.push_back(command);
			return *static_cast<Component*>(command.component_data);
		}

		template<typename Component>
		Component* get(const entity_id entity)
		{
			const auto& component_type = typeid(Component);

			// Scenario 1 (Component is in an archetype)
			auto archetype_it = entity_archetype_lookup.find(entity);
			if (archetype_it != entity_archetype_lookup.end())
			{
				if (archetype_it->second.contains(component_type))
				{
					archetype_id& memory_accesor = archetype_it->second;

					auto& block = blocks[memory_accesor];
					storage::region region = block.get_entity(entity);
					void* component_data = memory_accesor.get_component_ptr(component_type, region);
					return static_cast<Component*>(component_data);
				}
			}

			// Scenario 2 (Component is not in an archetype)
			for (const auto& add_command : add_commands)
			{
				if (add_command.target == entity && add_command.component_type == component_type)
				{
					return static_cast<Component*>(add_command.component_data);
				}
			}


			// Scenario 3 (Entity or component does not exist)
			return nullptr;
		}

		void emplace_commands();

		void execute_add(entity_id entity, void* data, std::type_index component_type, std::size_t component_size);
		void execute_remove(const remove_component_command& command);

		template<typename ... Components>
		void get_archetypes_with(std::vector<archetype_id>& archetypes)
		{
			for (auto& archetype_id : blocks | std::views::keys)
			{
				if ((archetype_id.contains(typeid(Components)) && ...))
				{
					archetypes.push_back(archetype_id);
				}
			}
		}

		template<typename Component>
		bool has(const entity_id entity)
		{
			auto archetype_it = entity_archetype_lookup.find(entity);
			if (archetype_it != entity_archetype_lookup.end())
			{
				return archetype_it->second.contains(typeid(Component));
			}

			return false;
		}

		template<typename Component>
		void remove(entity_id entity)
		{
			auto& id = entity_archetype_lookup[entity];

			if (!has<Component>(entity)) [[unlikely]]
				return;

			remove_component_command command = {
				.target = entity,
				.component_type = typeid(Component),
			};

			remove_commands.push_back(command);
		}

		template<typename ... Components>
		struct component_query
		{
			entity_id entity;
			archetype_pool& pool;
			std::vector<archetype_id> archetype_traversal;

			explicit component_query(archetype_pool& pool) : entity(-1),
			                                                 pool(pool),
			                                                 archetype_traversal()
			{
				pool.get_archetypes_with<Components...>(archetype_traversal);
			}

			class iterator
			{
			public:
				using iterator_category = std::forward_iterator_tag; // Iterator type_id (e.g., forward, bidirectional, etc.)
				using value_type = query_value;                                // Type of the elements
				using difference_type = std::ptrdiff_t;              // Type for representing differences between iterators
				using pointer = query_value*;                                  // Pointer to the element type_id
				using reference = query_value&;                                // Reference to the element type_id

				std::vector<archetype_id>::iterator archetype_it;
				std::vector<archetype_id>::iterator archetype_it_end;
				storage::iterator block_it;
				storage::iterator block_it_end;
				component_query& target_query;

				explicit iterator(component_query& query) :
					archetype_it(query.archetype_traversal.begin()),
					archetype_it_end(query.archetype_traversal.end()),
					block_it( archetype_it != archetype_it_end ? *query.pool.blocks[*archetype_it].begin() : *query.pool.empty_block.begin()),
					block_it_end(archetype_it != archetype_it_end ? *query.pool.blocks[*archetype_it].end() : *query.pool.empty_block.end()),
					target_query(query)
				{
					if (archetype_it == archetype_it_end)
					{
						return;
					}

					if (block_it == block_it_end)
					{
						++archetype_it;
						while (archetype_it != archetype_it_end)
						{
							block_it = *query.pool.blocks[*archetype_it].begin();
							block_it_end = *query.pool.blocks[*archetype_it].end();

							if (block_it != block_it_end)
							{
								break;
							}

							++archetype_it;
						}
					}

				}

				iterator(component_query& query,
					const std::vector<archetype_id>::iterator& archetype_it,
					const std::vector<archetype_id>::iterator& archetype_it_end, const storage::iterator& block_it,
					const storage::iterator& block_it_end) :
					archetype_it(archetype_it),
					archetype_it_end(archetype_it_end),
					block_it(block_it),
					block_it_end(block_it_end),
					target_query(query)
				{
				}

				iterator& operator++()
				{
					++block_it;
					if (block_it == block_it_end) [[unlikely]]
					{
						++archetype_it;
						while (archetype_it != archetype_it_end) [[unlikely]]
						{
							archetype_block& block = target_query.pool.blocks[*archetype_it];

							block_it = *block.begin();
							block_it_end = *block.end();

							if (block_it != block_it_end) [[likely]]
							{
								break;
							}

							++archetype_it;
						}
					}


					return *this;
				}

				bool operator==(const iterator& other) const
				{
					return archetype_it == other.archetype_it && block_it == other.block_it;
				}

				bool operator!=(const iterator& other) const
				{
					return archetype_it != other.archetype_it || block_it != other.block_it;
				}

				query_value operator*() const
				{
					return query_value{ *archetype_it, *block_it };
				}
			};


			iterator begin()
			{
				return iterator{ *this };
			}

			iterator end()
			{
				if (archetype_traversal.empty())
				{
					return iterator(*this,
						archetype_traversal.end(),
						archetype_traversal.end(),
						*pool.empty_block.end(),
						*pool.empty_block.end());
				}
				

				return  iterator(*this, 
					archetype_traversal.end(), 
					archetype_traversal.end(), 
					*pool.blocks[archetype_traversal.back()].end(), 
					*pool.blocks[archetype_traversal.back()].end());
			}
		};

		template<typename ... Components>
		component_query<Components...> query()
		{
			return component_query<Components...>(*this);
		}
	};
}