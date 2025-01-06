#pragma once
#include <typeindex>
#include <vector>

#include "storage.hpp"

namespace peetcs
{
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

	archetype_id();

	void init(std::type_index type, std::size_t type_size);

	bool contains(std::type_index type_id) const;

	std::size_t get_hash();

	std::size_t get_hash() const;

	void add_type(std::type_index type, std::size_t type_size);


	template <typename Type>
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

	template <typename Type>
	size_t get()
	{
		return get(typeid(Type));
	}

	template <typename Component>
	size_t byte_offset()
	{
		return byte_offset(typeid(Component));
	}

	size_t get(const std::type_index& type);

	size_t byte_offset(const std::type_index& type);

	size_t get_size_of_element() const;


	template <typename Component>
	void set_component(const Component& data, const storage::region& target)
	{
		size_t offset = byte_offset<Component>();
		std::memcpy(static_cast<storage::storage_type*>(target.data) + offset, &data, sizeof(Component));
	}

	template <typename Component>
	Component& get_component(const storage::region& target)
	{
		size_t offset = byte_offset<Component>();
		Component* component_ptr = reinterpret_cast<Component*>(static_cast<storage::storage_type*>(target.data) + offset);
		return *component_ptr;
	}


	void set_component_ptr(const std::type_index& type, const void* data, const storage::region& target);

	void* get_component_ptr(const std::type_index& type, const storage::region& target);

	void migrate(const storage::region& dst_region, const storage::region& src_region, archetype_id& src_memory_descriptor);

	bool operator==(const archetype_id& other) const;
};

	struct archetype_hash
	{
		size_t operator()(const archetype_id& obj) const
		{
			return obj.get_hash();
		}
	};
}