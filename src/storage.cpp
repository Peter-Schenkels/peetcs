#include "include/storage.hpp"

namespace peetcs
{
	peetcs::storage::storage() = default;

	storage::region storage::add_element()
	{
		auto element_region = region{ .index = element_index, .element_size = element_size, .data = &data[element_index * element_size] };
		element_index += 1;
		return element_region;
	}

	storage::region storage::get_element(const std::size_t index)
	{
		if (index >= element_index)
		{
			return region{ element_index, element_size, nullptr };
		}

		return region{ .index = index, .element_size = element_size, .data = &data[index * element_size] };
	}

	void storage::remove_element(std::size_t index)
	{
		if (index != last()) [[likely]]
		{
			std::memcpy(&data[index * element_size], &data[last() * element_size], element_size);
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
	{ return it_storage.get_element(current_element); }

	storage::region storage::iterator::operator->() const
	{ return it_storage.get_element(current_element); }

	bool storage::iterator::operator==(const iterator& other) const
	{ return current_element == other.current_element; }

	bool storage::iterator::operator!=(const iterator& other) const
	{ return current_element != other.current_element; }

	storage::iterator storage::begin()
	{
		return iterator{ *this, region{ 0, element_size, data.data() } };
	}

	storage::iterator storage::end()
	{
		return  iterator{ *this, region{ element_index, element_size, nullptr } };
	}

	storage::storage(std::size_t element_size, std::size_t number_of_elements): element_size(element_size)
	{
		data.resize(number_of_elements * element_size);
		element_index = 0;
	}
}