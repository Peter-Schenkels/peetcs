#pragma once
#include <typeindex>
#include <vector>

#include "peetcs_types.hpp"
#include "storage.hpp"

namespace peetcs
{
	class archetype_id
	{
		std::size_t hash = 0;
		size_t element_size = 0;
		bool dirty = true;

	public:
		std::vector<type_info> type_indices = {  };

		archetype_id();

		void init(std::type_index type, std::size_t type_size);

		bool contains(std::type_index type_id) const;

		std::size_t get_hash();

		std::size_t get_hash() const;

		void add_type(std::type_index type, std::size_t type_size);

		void remove_type(const std::type_index& type_id);

		template <typename Type>
		void remove_type()
		{
			const std::type_index type_id = std::type_index(typeid(Type));

			remove_type(type_id);
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
		static Component& get_component(const storage::region& target)
		{
			const std::size_t nb_of_types_in_element = *static_cast<uint8_t*>(target.storage_start);
			const std::size_t component_hash = typeid(Component).hash_code();
			const std::size_t base_offset = storage::padding_nb_of_element + nb_of_types_in_element * storage::padding_element_type;

			std::size_t element_offset = 0;

			for (size_t offset = 1; offset < (storage::padding_nb_of_element + nb_of_types_in_element * storage::padding_element_type);)
			{
				const uint8_t retrieved_hash_1 = *(static_cast<storage::storage_type*>(target.storage_start) + offset);

				if (retrieved_hash_1 == component_hash >> 3) [[unlikely]]
				{
					const uint8_t retrieved_hash_2 = *(static_cast<storage::storage_type*>(target.storage_start) + offset + 1);
					const uint8_t retrieved_hash_3 = *(static_cast<storage::storage_type*>(target.storage_start) + offset + 2);
					const uint8_t retrieved_hash_4 = *(static_cast<storage::storage_type*>(target.storage_start) + offset + 3);

					const uint64_t retrieved_hash = retrieved_hash_1 >> 3 * 8 | retrieved_hash_2 >> 2 * 8 | retrieved_hash_3 >> 1 * 8 | retrieved_hash_4;

					if (retrieved_hash == component_hash) [[likely]]
					{
						break;
					}
				}

				offset += 4;
				uint8_t element_size_1 = *(static_cast<storage::storage_type*>(target.storage_start) + offset);
				uint8_t element_size_2 = *(static_cast<storage::storage_type*>(target.storage_start) + offset + 1);

				element_offset += element_size_1 >> 8 | element_size_2;
			}

			Component* component_ptr = reinterpret_cast<Component*>(static_cast<storage::storage_type*>(target.data) + element_offset);
			return *component_ptr;
		}


		void set_component_ptr(const std::type_index& type, const void* data, const storage::region& target);

		void* get_component_ptr(const std::type_index& type, const storage::region& target);

		void migrate(const storage::region& dst_region, const storage::region& src_region, archetype_id& src_memory_descriptor);

		bool operator==(const archetype_id& other) const;
	};

	struct archetype_hash
	{
		size_t operator()(const archetype_id& obj) const;
		size_t operator()(const std::size_t& obj) const;
	};
}