#include "include/archetype_id.hpp"

#include <algorithm>

namespace peetcs
{
	archetype_id::archetype_id()
	{
		get_hash();
	}

	void archetype_id::init(std::type_index type, std::size_t type_size)
	{
		type_indices = { type_info{ type, type_size } };
		element_size = type_size;

		dirty = true;

		get_hash();
	}

	bool archetype_id::contains(std::type_index type_id) const
	{
		for (auto type_index : type_indices)
		{
			if (type_index.type_id == type_id)
			{
				return true;
			}
		}

		return false;
	}

	std::size_t archetype_id::get_hash()
	{
		if (dirty)
		{
			std::ranges::sort(type_indices, [](const type_info& a, const type_info& b) {
				return a.type_id.hash_code() < b.type_id.hash_code();
			});

			hash = 0;
			for (const auto& typeIndex : type_indices)
			{
				hash ^= std::hash<std::size_t>()(typeIndex.type_id.hash_code());
			}

			dirty = false;
		}

		return hash;
	}

	std::size_t archetype_id::get_hash() const
	{
		if (dirty)
		{
			__debugbreak();
		}

		return hash;
	}

	void archetype_id::add_type(std::type_index type, std::size_t type_size)
	{
		for (auto type_index : type_indices)
		{
			if (type_index.type_id == type)
			{
				return;
			}
		}

		type_indices.push_back({ type, type_size });
		element_size += type_size;

		dirty = true;
		get_hash();
	}

	void archetype_id::remove_type(const std::type_index& type_id)
	{
		auto it = type_indices.begin();
		for (; it != type_indices.end(); ++it)
		{
			if (it->type_id == type_id)
			{
				element_size -= it->size_of;
				break;
			}
		}

		if (it != type_indices.end())
		{
			type_indices.erase(it);
			dirty = true;
			get_hash();
		}
	}

	size_t archetype_id::get(const std::type_index& type)
	{
		if (dirty) [[unlikely]]
		{
			get_hash();
		}

		int index = 0;
		for (auto& type_index : type_indices)
		{
			if (type_index.type_id == type)
			{
				return index;
			}
			index++;
		}

		return -1;
	}

	size_t archetype_id::byte_offset(const std::type_index& type)
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

	size_t archetype_id::get_size_of_element() const
	{
		return element_size;
	}

	void archetype_id::set_component_ptr(const std::type_index& type, const void* data, const storage::region& target)
	{
		size_t offset = byte_offset(type);
		size_t size = type_indices[get(type)].size_of;

		std::memcpy(static_cast<storage::storage_type*>(target.data) + offset, data, size);
	}

	void* archetype_id::get_component_ptr(const std::type_index& type, const storage::region& target)
	{
		size_t offset = byte_offset(type);

		return static_cast<storage::storage_type*>(target.data) + offset;
	}

	void archetype_id::migrate(const storage::region& dst_region, const storage::region& src_region,
		archetype_id& src_memory_descriptor)
	{
		for (auto type_index_dst : type_indices)
		{
			for (auto type_index_src : src_memory_descriptor.type_indices)
			{
				if (type_index_dst.type_id == type_index_src.type_id)
				{
					if (type_index_dst.size_of != type_index_src.size_of)
					{
						__debugbreak();
					}

					set_component_ptr(type_index_dst.type_id, src_memory_descriptor.get_component_ptr(type_index_dst.type_id, src_region), dst_region);
				}
			}
		}
	}

	bool archetype_id::operator==(const archetype_id& other) const
	{
		if (dirty)
		{
			__debugbreak();
		}

		return hash == other.hash && hash == other.hash;
	}

	size_t archetype_hash::operator()(const archetype_id& obj) const
	{
		return obj.get_hash();
	}

	size_t archetype_hash::operator()(const std::size_t& obj) const
	{
		return obj;
	}
}