#pragma once
#include <unordered_map>
#include <ranges>
#include "generic_container.hpp"
#include "query_value.hpp"


namespace peetcs
{
	using entity_id      = element_layout::id_t;
	using component_id   = element_layout::index_t;
	using component_type = element_layout::hash_t;

	constexpr entity_id invalid_entity_id = std::numeric_limits<entity_id>::max();
	constexpr entity_id invalid_component_id = std::numeric_limits<component_id>::max();


	struct add_component_command
	{
		entity_id      target;
		component_type component_type;
		std::size_t    component_size;
		void*          component_data;
	};

	struct remove_component_command
	{
		entity_id      target;
		component_type component_type;
		component_id   list_index;
	};

	struct archetype_pool
	{
		std::unordered_map<element_layout, generic_container> blocks;
		std::unordered_map<entity_id, element_layout>         entity_archetype_lookup;
		std::vector<add_component_command>                    add_commands;
		std::vector<remove_component_command>                 remove_commands;
		// used for storing component lists contiguously
		std::unordered_map<entity_id, std::unordered_map<component_type, generic_container>> list_blocks;

		std::size_t default_array_size = 10000;



		archetype_pool()
		{
			add_commands.reserve(default_array_size * 10);
			remove_commands.reserve(default_array_size * 10);
		}

		template<typename Component>
		Component& add(const entity_id entity)
		{
			add_component_command command = {
				.target = entity,
				.component_type = Component::id,
				.component_size = sizeof(Component),
				.component_data = malloc(sizeof(Component))
			};

			*static_cast<Component*>(command.component_data) = Component{};

			add_commands.push_back(command);

			return *static_cast<Component*>(command.component_data);
		}

		template<typename Component>
		Component& emplace(const entity_id entity)
		{
			auto rep = execute_add(entity, nullptr, Component::id, sizeof(Component));
			return *rep.template get_ptr<Component>();
		}

		template<typename Component>
		Component* get_from_owner(const entity_id entity)
		{
			return get_at<Component>(entity, 0);
		}

		template<typename Component>
		generic_container* get_list_container(const entity_id entity)
		{
			const auto& component_type = Component::id;
			auto entity_containers_it = list_blocks.find(entity);
			if (entity_containers_it != list_blocks.end())
			{
				auto component_list_it = entity_containers_it->second.find(Component::id);
				if (component_list_it != entity_containers_it->second.end())
				{
					return &component_list_it->second;
				}
			}

			return {};
		}

		template<typename Component>
		bool has_list(const entity_id entity)
		{
			return get_list_container<Component>(entity) != nullptr;
		}

		template<typename Component>
		Component* get_at(const entity_id entity, const component_id index)
		{
			const auto& component_type = Component::id;

			// Scenario 1 (Component is in an archetype)
			auto archetype_it = entity_archetype_lookup.find(entity);
			if (archetype_it != entity_archetype_lookup.end())
			{
				if (archetype_it->second.contains(component_type))
				{
					// Find component list
					if (index > 0)
					{
						auto entity_containers_it = list_blocks.find(entity);
						if ( entity_containers_it != list_blocks.end())
						{
							auto component_list_it = entity_containers_it->second.find(Component::id);
							if (component_list_it != entity_containers_it->second.end())
							{
								component_id index_in_list = index - 1;
								if (component_list_it->second.size() > index_in_list)
								{
									return component_list_it->second.get_element_at(index_in_list).template get_ptr<Component>();
								}
							}
						}
					}
					// Component is stored inside the archetype container
					else [[likely]]
					{
						auto& block = blocks[archetype_it->second];
						generic_container::element_rep rep = block.get_element(entity);
						return rep.get_ptr<Component>();
					}
				}
			}

			// Scenario 2 (Component is not in an archetype)
			component_id current_type_index = 0;
			for (const auto& add_command : add_commands)
			{
				if (add_command.target == entity && add_command.component_type == component_type)
				{
					if (index == current_type_index)
					{
						return static_cast<Component*>(add_command.component_data);
					}

					// Try to match in cases when we add a component list while emplacing
					current_type_index++;
				}
			}

			// Scenario 3 (Entity or component does not exist)
			return nullptr;
		}

		void emplace_commands();

		generic_container::element_rep execute_add(entity_id entity, void* data, component_type component_type, std::size_t component_size);
		void execute_remove(const remove_component_command& command);

		template<typename ... Components>
		void get_archetypes_with(std::vector<element_layout>& archetypes)
		{
			for (auto& element_layout : blocks | std::views::keys)
			{
				if ((element_layout.contains(Components::id) && ...))
				{
					archetypes.push_back(element_layout);
				}
			}
		}

		template<typename Component>
		bool has(const entity_id entity)
		{
			auto archetype_it = entity_archetype_lookup.find(entity);
			if (archetype_it != entity_archetype_lookup.end())
			{
				return archetype_it->second.contains(Component::id);
			}

			return false;
		}

		template<typename Component>
		void remove_component(entity_id entity)
		{
			remove_component_at<Component>(entity, 0);
		}

		template<typename Component>
		void remove_component_at(entity_id entity, component_id index)
		{
			auto& id = entity_archetype_lookup[entity];

			if (!id.contains(Component::id)) [[unlikely]]
				return;

			remove_component_command command = {
				.target = entity,
				.component_type = Component::id,
				.list_index = index
			};

			remove_commands.push_back(command);
		}

		template<typename ... Components>
		struct component_query
		{
			entity_id entity;
			archetype_pool& pool;
			std::vector<element_layout> archetype_traversal;

			explicit component_query(archetype_pool& pool) : entity(-1),
			                                                 pool(pool),
			                                                 archetype_traversal()
			{
				pool.get_archetypes_with<Components...>(archetype_traversal);
			}

			class iterator
			{
			public:
				using iterator_category = std::forward_iterator_tag;
				using value_type = query_value;                                
				using difference_type = std::ptrdiff_t; 
				using pointer = query_value*;                                  
				using reference = query_value&;                                

				std::vector<element_layout>::iterator archetype_it;
				std::vector<element_layout>::iterator archetype_it_end;
				generic_container::iterator block_it;
				generic_container::iterator block_it_end;
				component_query& target_query;

				explicit iterator(component_query& query) :
					archetype_it(query.archetype_traversal.begin()),
					archetype_it_end(query.archetype_traversal.end()),
					block_it( archetype_it != archetype_it_end ? query.pool.blocks[*archetype_it].begin() : query.pool.blocks[element_layout{}].begin()),
					block_it_end(archetype_it != archetype_it_end ? query.pool.blocks[*archetype_it].end() : query.pool.blocks[element_layout{}].end()),
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
							new (&block_it) generic_container::iterator(query.pool.blocks[*archetype_it].begin());
							new (&block_it_end) generic_container::iterator(query.pool.blocks[*archetype_it].end());

							if (block_it != block_it_end)
							{
								break;
							}

							++archetype_it;
						}
					}

				}

				iterator(component_query& query,
					const std::vector<element_layout>::iterator& archetype_it,
					const std::vector<element_layout>::iterator& archetype_it_end, const generic_container::iterator& block_it,
					const generic_container::iterator& block_it_end) :
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
							generic_container& block = target_query.pool.blocks[*archetype_it];

							new (&block_it) generic_container::iterator(block.begin());
							new (&block_it_end) generic_container::iterator(block.end());

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
				if (archetype_traversal.empty())
				{
					pool.blocks.try_emplace(element_layout{});

					return iterator(*this,
						archetype_traversal.end(),
						archetype_traversal.end(),
						pool.blocks[element_layout{}].end(),
						pool.blocks[element_layout{}].end());
				}

				return iterator{ *this };
			}

			iterator end()
			{
				if (archetype_traversal.empty())
				{
					return iterator(*this,
						archetype_traversal.end(),
						archetype_traversal.end(),
						pool.blocks[element_layout{}].end(),
						pool.blocks[element_layout{}].end());
				}

				return iterator(*this, 
					archetype_traversal.end(), 
					archetype_traversal.end(), 
					pool.blocks[archetype_traversal.back()].end(), 
					pool.blocks[archetype_traversal.back()].end());
			}
		};

		template<typename ... Components>
		component_query<Components...> query()
		{
			return component_query<Components...>(*this);
		}
	};
}
