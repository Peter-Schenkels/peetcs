#include "include/storage.hpp"

#include "include/peetcs_types.hpp"

namespace peetcs
{
	peetcs::storage::storage() = default;

	storage::region storage::add_element(identifier_type identifier)
	{
		uint8_t* id_memory_start = &data[padding_index_end + element_index * (element_size + padding_element_id)];
		std::memcpy(id_memory_start, &identifier, sizeof(identifier_type));

		auto element_region = region {
			.index = element_index,
			.element_size = element_size + padding_element_id,
			.identifier = identifier,
			.data = &data[padding_index_end + element_index * (element_size + padding_element_id) + padding_element_id],
			.storage_start = data.data()
		};

		element_index += 1;
		return element_region;
	}

	storage::region storage::get_element(const std::size_t index)
	{
		if (index >= element_index)
		{
			return region {
				.index= element_index,
				.element_size= element_size,
				.identifier = INT32_MIN,
				.data= nullptr,
				.storage_start= nullptr,
			};
		}

		int identifier;
		std::memcpy(&identifier, &data[padding_index_end + index * (element_size + padding_element_id)], sizeof(identifier_type));

		return region{
			.index = index,
			.element_size = element_size,
			.identifier = identifier,
			.data = &data[padding_index_end + index * (element_size + padding_element_id) + padding_element_id],
			.storage_start = data.data()
		};
	}

	void storage::remove_element(const std::size_t index)
	{
		if (index != last()) [[likely]]
		{
			std::memcpy(&data[index * element_size], &data[padding_index_end + last() * element_size], element_size);
		}

		element_index--;
	}

	std::size_t storage::size() const
	{
		return data.size() / element_size;
	}

	std::size_t storage::last() const
	{
		return element_index - 1;
	}

	storage::iterator::iterator(const iterator& it):
		current_element(it.current_element),
		it_storage(it.it_storage)
	{
	}

	storage::iterator::iterator(storage& storage, const region& start):
		current_element(start.index),
		it_storage(storage)
	{
	}

	storage::iterator& storage::iterator::operator++()
	{
		current_element++;
		return *this;
	}

	storage::iterator& storage::iterator::operator=(const iterator& other)
	{
		current_element = other.current_element;
		new (&it_storage) storage(other.it_storage);

		return *this;
	}

	storage::region storage::iterator::operator*() const
	{
		return it_storage.get_element(current_element);
	}

	storage::region storage::iterator::operator->() const
	{
		return it_storage.get_element(current_element);
	}

	bool storage::iterator::operator==(const iterator& other) const
	{
		return current_element == other.current_element;
	}

	bool storage::iterator::operator!=(const iterator& other) const
	{
		return current_element != other.current_element;
	}

	storage::iterator storage::begin()
	{
		return iterator {
			*this, get_element(0)
		};
	}

	storage::iterator storage::end()
	{
		return  iterator {
			*this, get_element(std::numeric_limits<std::size_t>::max())
		};
	}

	storage::storage(const std::vector<type_info>& type_description, const std::size_t number_of_elements)
	{
		if (type_description.size() > std::numeric_limits<uint8_t>::max())
		[[unlikely]]
		{
			__debugbreak();
		}

		uint8_t nb_of_sub_types_in_element = static_cast<uint8_t>(type_description.size());


		uint8_t descriptor_padding_size = padding_nb_of_element;
		descriptor_padding_size += nb_of_sub_types_in_element * padding_element_type; // 4 for type - 2 for element_size

		element_size = 0;
		for (const auto& description : type_description)
		{
			element_size += description.size_of;
		}

		padding_index_end = descriptor_padding_size;
		element_index = 0;

		data.resize(descriptor_padding_size + number_of_elements * element_size);
		data.emplace_back(nb_of_sub_types_in_element);

		for (const auto& [type_id, type_size] : type_description)
		{
			const uint64_t type_hash = type_id.hash_code();
			uint8_t type_id_layout_1 = static_cast<uint8_t>(type_hash >> 3);
			uint8_t type_id_layout_2 = static_cast<uint8_t>(type_hash >> 2);
			uint8_t type_id_layout_3 = static_cast<uint8_t>(type_hash >> 1);
			uint8_t type_id_layout_4 = static_cast<uint8_t>(type_hash >> 0);

			if (type_size >= std::numeric_limits<uint16_t>::max())
			[[unlikely]]
			{
				__debugbreak();
			}

			const uint16_t sub_element_size = static_cast<uint16_t>(type_size);
			uint8_t sub_element_size_1 = static_cast<uint8_t>(sub_element_size >> 1);
			uint8_t sub_element_size_2 = static_cast<uint8_t>(sub_element_size >> 0);

			data.insert(data.end(), {
				            type_id_layout_1, type_id_layout_2, type_id_layout_3, type_id_layout_4, sub_element_size_1,
				            sub_element_size_2
			            });
		}
	}
}