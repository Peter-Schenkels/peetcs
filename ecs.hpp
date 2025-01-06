#pragma once
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <ranges>
#include <type_traits>
#include <iostream>
#include <memory>
#include <cstring>
#include <optional>
#include <algorithm>



typedef int entity_id;

class storage
{
public:
	typedef std::uint8_t storage_type;

private:
	std::vector<storage_type> data;

	std::size_t element_size;
	std::size_t element_index;

public:
	struct region
	{
		size_t index;
		std::size_t element_size;
		void* data;
	};

	storage() = default;
	storage(std::size_t element_size, std::size_t number_of_elements) : element_size(element_size)
	{
		data.resize(number_of_elements * element_size);
		element_index = 0;
	}

	region add_element()
	{
		auto element_region = region{ .index= element_index, .element_size= element_size, .data= &data[element_index * element_size]};
		element_index += 1;
		return element_region;
	}

	region get_element(const std::size_t index)
	{
		if (index >= element_index)
		{
			return region{element_index, element_size, nullptr};
		}

		return region{ .index = index, .element_size = element_size, .data = &data[index * element_size] };
	}

	void remove_element(std::size_t index) 
	{
		if (index != last()) [[likely]]
		{
			std::memcpy(&data[index * element_size], &data[last() * element_size], element_size);
		}

		element_index--;
	}

	[[nodiscard]] std::size_t size() const
	{
		return data.size() / element_size;
	}

	[[nodiscard]] std::size_t last() const
	{
		return element_index - 1;
	}


	class iterator
	{
		public:
			using iterator_category = std::forward_iterator_tag; // Iterator type (e.g., forward, bidirectional, etc.)
			using value_type = region;                                // Type of the elements
			using difference_type = std::ptrdiff_t;              // Type for representing differences between iterators
			using pointer = region*;                                  // Pointer to the element type
			using reference = region&;                                // Reference to the element type

		std::size_t current_element = 0;
		storage& it_storage;

		iterator(const iterator& it) :
			current_element(it.current_element),
			it_storage(it.it_storage)
		{}

		iterator(storage& storage, const region& start) :
			current_element(start.index),
			it_storage(storage)
		{
		}

		iterator& operator++() {
			current_element++;
			return *this;
		}

		iterator& operator=(const iterator& other)
		{
			current_element = other.current_element;
			new (&it_storage) storage(other.it_storage);

			return *this;
		}

		region operator*() const { return it_storage.get_element(current_element); }

		region operator->() const { return it_storage.get_element(current_element); }


		bool operator==(const iterator& other) const { return current_element == other.current_element; }

		// Inequality comparison
		bool operator!=(const iterator& other) const { return current_element != other.current_element; }
	};


	iterator begin()
	{
		return iterator{*this, region{ 0, element_size, data.data() }};
	}

	iterator end()
	{
		return  iterator{*this, region{ element_index, element_size, nullptr }};
	}
};

class archetype_id
{
	struct type_info_size
	{
		std::type_index type;
		size_t size_of;
	};

	std::size_t hash = 0;
	size_t element_size = 0;
	bool dirty = true;

public:
	std::vector<type_info_size> type_indices = {  };

	archetype_id()
	{
		get_hash();
	}

	void init(std::type_index type, std::size_t type_size)
	{
		type_indices = { type_info_size{ type, type_size } };
		element_size = type_size;

		dirty = true;

		get_hash();
	}

	bool contains(std::type_index type_id) const
	{
		for (auto type_index : type_indices)
		{
			if (type_index.type == type_id)
			{
				return true;
			}
		}

		return false;
	}

	std::size_t get_hash()
	{
		if (dirty)
		{
			std::ranges::sort(type_indices, [](const type_info_size& a, const type_info_size& b) {
				return a.type.hash_code() < b.type.hash_code();
				});

			hash = 0;
			for (const auto& typeIndex : type_indices)
			{
				hash ^= std::hash<std::size_t>()(typeIndex.type.hash_code());
			}

			dirty = false;
		}

		return hash;
	}

	std::size_t get_hash() const
	{
		if (dirty)
		{
			__debugbreak();
		}

		return hash;
	}

	void add_type(std::type_index type, std::size_t type_size)
	{
		for (auto type_index : type_indices)
		{
			if (type_index.type == type)
			{
				return;
			}
		}

		type_indices.push_back({ type, type_size });
		element_size += type_size;

		dirty = true;
		get_hash();
	}

	template<typename Type>
	void remove_type()
	{
		const auto type_id = std::type_index(typeid(Type));

		for (auto it = type_indices.begin(); it != type_indices.end(); ++it)
		{
			if (it->type == type_id)
			{
				type_indices.erase(it);
				element_size -= sizeof(Type);
				dirty = true;
				get_hash();
				return;
			}
		}
	}

	template<typename Type>
	size_t get()
	{
		return get(typeid(Type));
	}

	template<typename Component>
	size_t byte_offset()
	{
		return byte_offset(typeid(Component));
	}

	size_t get(const std::type_index& type)
	{
		if (dirty) [[unlikely]]
		{
			get_hash();
		}

		int index = 0;
		for (auto& type_index : type_indices)
		{
			if (type_index.type == type)
			{
				return index;
			}
			index++;
		}

		return -1;
	}

	size_t byte_offset(const std::type_index& type)
	{
		if (dirty) [[unlikely]]
		{
			get_hash();
		}

		size_t component_index = get(type);

		size_t byte = 0;
		for (size_t index = 0; index < component_index; ++index)
		{
			byte += type_indices[index].size_of;
		}

		return byte;
	}

	size_t get_size_of_element() const
	{
		return element_size;
	}

	template<typename Component>
	void set_component(const Component& data, const storage::region& target)
	{
		size_t offset = byte_offset<Component>();
		std::memcpy(static_cast<storage::storage_type*>(target.data) + offset, &data, sizeof(Component));
	}

	template<typename Component>
	Component& get_component(const storage::region& target)
	{
		size_t offset = byte_offset<Component>();
		Component* component_ptr = reinterpret_cast<Component*>(static_cast<storage::storage_type*>(target.data) + offset);
		return *component_ptr;
	}

	void set_component_ptr(const std::type_index& type, const void* data, const storage::region& target)
	{
		size_t offset = byte_offset(type);
		size_t size = type_indices[get(type)].size_of;

		std::memcpy(static_cast<storage::storage_type*>(target.data) + offset, data, size);
	}

	void* get_component_ptr(const std::type_index& type, const storage::region& target)
	{
		size_t offset = byte_offset(type);

		return static_cast<storage::storage_type*>(target.data) + offset;
	}

	void migrate(const storage::region& dst_region, const storage::region& src_region, archetype_id& src_memory_descriptor)
	{
		for (auto type_index_dst : type_indices)
		{
			for (auto type_index_src : src_memory_descriptor.type_indices)
			{
				if (type_index_dst.type == type_index_src.type)
				{
					if (type_index_dst.size_of != type_index_src.size_of)
					{
						__debugbreak();
					}

					set_component_ptr(type_index_dst.type, src_memory_descriptor.get_component_ptr(type_index_dst.type, src_region), dst_region);
				}
			}
		}
	}

	bool operator==(const archetype_id& other) const
	{
		if (dirty)
		{
			__debugbreak();
		}

		return hash == other.hash && hash == other.hash;
	}
};

struct archetype_hash
{
	size_t operator()(const archetype_id& obj) const
	{
		return obj.get_hash();
	}
};

class archetype_block
{
	storage block;
	archetype_id archetype_info;

public:

	archetype_block() = default;

	explicit archetype_block(const archetype_id& id):
		block(id.get_size_of_element(), 1000),
		archetype_info(id)
	{
	}

	storage::region add_entity(const entity_id entity)
	{
		auto region = block.add_element();
		entity_index_map[entity] = region.index;

		return region;
	}

	storage::region get_entity(const entity_id entity)
	{
		return block.get_element(entity_index_map[entity]);
	}

	void remove_entity(const entity_id entity)
	{
		const auto entity_it = entity_index_map.find(entity);
		if (entity_it == entity_index_map.end())
		{
			__debugbreak();
		}

		size_t last_entity = block.last();
		block.remove_element(entity_it->second);
		entity_index_map.erase(entity_it);

		for (auto& element : entity_index_map)
		{
			// Move last to deleted entity index
			if (element.second == last_entity)
			{
				entity_index_map[element.first] = entity_it->second;
			}
		}
	}

	std::unordered_map<entity_id, std::size_t> entity_index_map;

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag; // Iterator type (e.g., forward, bidirectional, etc.)
		using value_type = storage::iterator;                                // Type of the elements
		using difference_type = std::ptrdiff_t;              // Type for representing differences between iterators
		using pointer = storage::iterator*;                                  // Pointer to the element type
		using reference = storage::iterator&;                                // Reference to the element type

		storage::iterator it;


		explicit iterator(const storage::iterator& it) :
			it(it)
		{
		}

		iterator& operator++() {
			++it;
			return *this;
		}

		storage::iterator operator*() const { return it; }
		storage::iterator operator->() const { return it; }

		bool operator==(const iterator& other) const { return it == other.it; }

		// Inequality comparison
		bool operator!=(const iterator& other) const { return it != other.it; }
	};

	iterator begin()
	{
		return iterator{block.begin()};
	}

	iterator end()
	{
		return iterator{block.end()};
	}
};

struct add_component_command
{
	entity_id target;
	std::type_index component_type;
	std::size_t component_size;
	void* component_data;
};



struct query_value
{
	archetype_id& memory_accesor;
	storage::region region;

	query_value(archetype_id& memory_accesor, const storage::region& region) :
		memory_accesor(memory_accesor),
		region(region)
	{
	}

	template<typename Component>
	Component& get()
	{
		return memory_accesor.get_component<Component>(region);
	}
};




struct archetype_pool
{


	std::unordered_map<archetype_id, archetype_block, archetype_hash> blocks;
	std::unordered_map<entity_id, archetype_id> entity_archetype_lookup;
	std::unordered_map<entity_id, std::unordered_map<std::type_index, std::vector<add_component_command>>> add_commands;

	template<typename Component>
	Component& add(entity_id entity)
	{
		add_component_command command = {
			entity,
			typeid(Component),
			sizeof(Component),
			malloc(sizeof(Component))
		};

		*static_cast<Component*>(command.component_data) = Component{};

		add_commands[entity][command.component_type].push_back(command);
		return *static_cast<Component*>(add_commands[entity][command.component_type].back().component_data);
	}

	template<typename Component>
	Component* get(entity_id entity)
	{
		// Scenario 1 (Component is in an archetype)
		auto archetype_it = entity_archetype_lookup.find(entity);
		if (archetype_it != entity_archetype_lookup.end())
		{
			if (archetype_it->second.contains(typeid(Component)))
			{
				archetype_id& memory_accesor = archetype_it->second;

				auto& block = blocks[memory_accesor];
				storage::region region = block.get_entity(entity);
				void* component_data = memory_accesor.get_component_ptr(typeid(Component), region);
				return static_cast<Component*>(component_data);
			}
		}

		// Scenario 2 (Component is not in an archetype)
		auto entity_command_it = add_commands.find(entity);
		if (entity_command_it != add_commands.end())
		{
			auto component_command_it = entity_command_it->second.find(typeid(Component));
			if (component_command_it != entity_command_it->second.end())
			{
				auto& commands = component_command_it->second;
				if (!commands.empty())
				{
					return static_cast<Component*>(commands.front().component_data);
				}
			}
		}

		// Scenario 3 (Entity or component does not exist)
		return nullptr;
	}

	void emplace_commands()
	{
		for (auto& entity_commands: add_commands | std::views::values)
		{
			for (auto& component_commands : entity_commands | std::views::values)
			{
				for (auto add_command : component_commands)
				{
					execute_add(add_command.target, add_command.component_data, add_command.component_type, add_command.component_size);
					free(add_command.component_data);

				}
			}
		}
	}

	void execute_add(entity_id entity, void* data, std::type_index component_type, std::size_t component_size)
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

				blocks[memory_descriptor_new] = archetype_block{memory_descriptor_new};
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

	template<typename ... Components>
	struct component_query
	{
		entity_id entity;
		archetype_pool& pool;
		std::vector<archetype_id> archetype_traversal;

		explicit component_query(archetype_pool& pool) :
			pool(pool),
			archetype_traversal()
		{
			pool.get_archetypes_with<Components...>(archetype_traversal);
		}

		class iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag; // Iterator type (e.g., forward, bidirectional, etc.)
			using value_type = query_value;                                // Type of the elements
			using difference_type = std::ptrdiff_t;              // Type for representing differences between iterators
			using pointer = query_value*;                                  // Pointer to the element type
			using reference = query_value&;                                // Reference to the element type

			std::vector<archetype_id>::iterator archetype_it;
			std::vector<archetype_id>::iterator archetype_it_end;

			storage::iterator block_it;
			storage::iterator block_it_end;

			component_query& target_query;

			iterator(component_query& query) :
				archetype_it(query.archetype_traversal.begin()),
				archetype_it_end(query.archetype_traversal.end()),
				block_it(*query.pool.blocks[*archetype_it].begin()),
				block_it_end(*query.pool.blocks[*archetype_it].end()),
				target_query(query)
			{
			}

			iterator(component_query& query,
				const std::vector<archetype_id>::iterator& archetype_it,
				const std::vector<archetype_id>::iterator& archetype_it_end,
				const storage::iterator& block_it,
				const storage::iterator& block_it_end)
				:
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

			query_value operator*() const { return query_value{ *archetype_it, *block_it }; }

			bool operator==(const iterator& other) const { return archetype_it == other.archetype_it && block_it == other.block_it; }

			// Inequality comparison
			bool operator!=(const iterator& other) const { return archetype_it != other.archetype_it || block_it != other.block_it; }
		};


		iterator begin()
		{
			return iterator{ *this };
		}

		iterator end()
		{
			return  iterator( *this, archetype_traversal.end(), archetype_traversal.end(), *pool.blocks[archetype_traversal.back()].end(), *pool.blocks[archetype_traversal.back()].end() );
		}
	};

	template<typename ... Components>
	component_query<Components...> query()
	{
		return component_query<Components...>(*this);
	}
};


