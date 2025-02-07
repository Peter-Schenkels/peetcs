#include "include/generic_container.hpp"

#include <algorithm>
#include <iostream>
bool element_layout::element_info::operator==(const element_info& element_info) const = default;

void element_layout::add(const hash_t& id, const stride_t& stride)
{
	for (auto type_info : layout)
	{
		if (type_info.id == id)
		{
			return;
		}
	}

	layout.emplace_back(id, stride);
	element_size += stride + sizeof(sub_element_info);
	meta_size += sizeof(type_info);

	cached_hash = hash();
}

bool element_layout::contains(const hash_t& type) const
{
	for (auto type_info : layout)
	{
		if (type_info.id == type)
		{
			return true;
		}
	}

	return false;
}

void element_layout::remove(const hash_t type_index)
{
	type_info found_element = type_info{ 0, 0 };
	if (!erase_if(layout, [&](const type_info& element)
		{
			if (element.id == type_index)
			{
				found_element = element;
				return true;
			}
			return false;
		})) [[unlikely]]
	{
		return;
	}

	element_size -= found_element.stride + sizeof(sub_element_info);
	meta_size += sizeof(type_info);

	cached_hash = hash();
}

std::size_t element_layout::hash()
{
	std::ranges::sort(layout, [](const type_info& a, const type_info& b) {
		return a.id < b.id;
	});

	std::size_t result = 0;
	for (const auto& type_index : layout)
	{
		result ^= std::hash<std::size_t>()(type_index.id);
	}

	// Optionally include the element_size and meta_size
	result ^= std::hash<std::size_t>()(element_size) << 2;
	result ^= std::hash<std::size_t>()(meta_size) << 3;

	return result;
}

void* element_layout::get_sub_element(hash_t type, void* element_ptr)
{
	if (element_ptr == nullptr)
	{
		halt();
	}

	const element_info* info = reinterpret_cast<element_info*>(element_ptr);

	uint8_t* sub_element_ptr = static_cast<uint8_t*>(element_ptr) + sizeof(element_info);

	int i = sub_element_ptr - element_ptr;

	std::size_t offset = 0;
	std::size_t max_offset = info->stride - sizeof(sub_element_info);

	while (offset < max_offset) [[likely]]
	{
		const auto sub_element = reinterpret_cast<const sub_element_info*>(sub_element_ptr + offset);
		if (sub_element->id == type)
		{
			return sub_element_ptr + offset + sizeof(sub_element_info);
		}

		offset += sizeof(sub_element_info) + sub_element->stride;
	}

	return nullptr;
}

generic_container::element_rep::element_rep(void* element_ptr, generic_container& parent):
	parent_container(parent),
	value_at_pointer(element_ptr == nullptr ? element_layout::element_info{} : *static_cast<element_layout::element_info*>(element_ptr)),
	element_ptr(element_ptr)
{
}

void generic_container::element_rep::verify_ptr()
{
	// Check if our pointer is still valid
	if (*static_cast<element_layout::element_info*>(element_ptr) != value_at_pointer) [[unlikely]]
	{
		element_rep rep  = parent_container.get_element_at(value_at_pointer.index);
		value_at_pointer = rep.value_at_pointer;
		element_ptr      = rep.element_ptr;
	}
}

element_layout::id_t generic_container::element_rep::get_id() const
{
	return value_at_pointer.element_id;
}

element_layout::index_t generic_container::element_rep::get_index() const
{
	return value_at_pointer.index;
}

void generic_container::element_rep::set_generic_ptr(const element_layout::hash_t type, const std::size_t stride,
	const void* new_data)
{
	if (new_data == nullptr)
	{
		return;
	}

	std::memcpy(get_ptr_generic(type), new_data, stride);
}

void* generic_container::element_rep::get_ptr_generic(const element_layout::hash_t type)
{
	verify_ptr();

	void* sub_element_ptr = element_layout::get_sub_element(type, static_cast<uint8_t*>(element_ptr));
	return sub_element_ptr;
}

void generic_container::element_rep::copy_sub_elements(const element_rep& old_region) const
{
	const element_layout::element_info* info = static_cast<element_layout::element_info*>(element_ptr);

	uint8_t* sub_element_ptr = static_cast<uint8_t*>(element_ptr) + sizeof(element_layout::element_info);
	std::size_t offset = 0;
	std::size_t max_offset = info->stride - sizeof(element_layout::sub_element_info);

	while (offset < max_offset) [[likely]]
	{
		const auto sub_element = reinterpret_cast<const element_layout::sub_element_info*>(sub_element_ptr + offset);
		void* old_sub_element_ptr = element_layout::get_sub_element(element_layout::hash_t{sub_element->id}, old_region.element_ptr);

		if (old_sub_element_ptr != nullptr)
		{
			std::memcpy(sub_element_ptr + sizeof(element_layout::sub_element_info) + offset, old_sub_element_ptr, sub_element->stride);
		}

		offset += sizeof(element_layout::sub_element_info) + sub_element->stride;
	}
}

#ifndef DEPRECATED
generic_container::generic_container(const std::vector<element_layout::type_info>& types, element_layout::index_t nb_of_elements)
{
	layout = {};

	for (auto type : types)
	{
		layout.add(type.id, type.stride);
	}

	*this = generic_container(layout, nb_of_elements);
}
#endif

generic_container::generic_container(element_layout in_layout, element_layout::index_t nb_of_elements):
	data(),
	layout(std::move(in_layout)),
	layout_hash(layout.hash()),
	max(nb_of_elements),
	element_stack_index(0)
{
	data.resize(get_byte_index(nb_of_elements));
	std::memcpy(data.data(), layout.layout.data(), layout.meta_size);
}

std::size_t generic_container::get_byte_index(const element_layout::index_t element_index) const
{
	return layout.meta_size + element_index * layout.element_size;
}

generic_container::element_rep generic_container::add_element(const element_layout::id_t id)
{
	if (element_stack_index >= (max - 1)) [[unlikely]]
	{
		resize(max * 2);
	}

	uint8_t* data_ptr = data.data();

	// Create first element in cache unfriendly way
	if (element_stack_index == 0) [[unlikely]]
	{
		std::size_t byte_offset = get_byte_index(element_stack_index);
		const auto element_info = reinterpret_cast<element_layout::element_info*>(data_ptr + byte_offset);
		element_info->element_id = id;
		element_info->index = element_stack_index;
		element_info->stride = layout.element_size;

		// initialise all members
		byte_offset += sizeof(element_layout::element_info);
		for (auto type_info : layout.layout)
		{
			const auto sub_element_info = reinterpret_cast<element_layout::sub_element_info*>(data_ptr + byte_offset);
			sub_element_info->id        = type_info.id;
			sub_element_info->stride    = type_info.stride;

			byte_offset += sizeof(element_layout::sub_element_info) + sub_element_info->stride;
		}

		element_stack_index++;
		return element_rep{ element_info, *this };
	}
	// All following elements are direct copies of the previous element's data (metadata not included)
	else [[likely]]
	{
		std::size_t byte_offset = get_byte_index(element_stack_index);
		std::size_t prev_byte_offset = get_byte_index(element_stack_index - 1);

		std::memcpy(data_ptr + byte_offset, data_ptr + prev_byte_offset, layout.element_size);
		const auto  element_info = reinterpret_cast<element_layout::element_info*>(data_ptr + byte_offset);
		element_info->element_id = id;
		element_info->index = element_stack_index;

		element_stack_index++;
		return element_rep{ element_info, *this };
	}
}

generic_container::element_rep generic_container::get_element_at(const element_layout::index_t index)
{
	if (index >= max || index >= element_stack_index) [[unlikely]]
	{
		halt();
	}

	const std::size_t byte_offset = get_byte_index(index);

	return element_rep{ data.data() + byte_offset, *this };
}

generic_container::element_rep generic_container::get_element(const element_layout::id_t id)
{
	auto test = begin();
	auto test_e = end();

	int i = 0;

	for (element_rep element : *this)
	{
		i++;
		if (element.get_id() == id)
		{
			return element;
		}
	}

	return element_rep{ nullptr, *this };
}

bool generic_container::contains(const element_layout::id_t id)
{
	return get_element(id).element_ptr != nullptr;
}

void generic_container::remove_at(const element_layout::index_t index)
{
	if (index >= max || index >= element_stack_index) [[unlikely]]
	{
		halt();
	}

	if (element_stack_index - 1 == index)
	{
		element_stack_index--;
		return;
	}


	void* remove_ptr = data.data() + get_byte_index(index);
	void* last_ptr   = data.data() + get_byte_index(element_stack_index - 1);

	std::memcpy(remove_ptr, last_ptr, layout.element_size);
	element_layout::element_info* info = static_cast<element_layout::element_info*>(remove_ptr);
	info->index = index;
	element_stack_index--;
}

void generic_container::remove(const element_layout::id_t id)
{
	element_rep element = get_element(id);
	if (element.element_ptr == nullptr)
	{
		return;
	}

	remove_at(element.get_index());
}

void generic_container::resize(std::size_t new_size)
{
	max = new_size;
	data.resize(get_byte_index(new_size));
}

generic_container::iterator::iterator(const iterator& it) = default;

generic_container::iterator::iterator(generic_container& container, element_layout::index_t start):
	current_element(start),
	it_container(container)
{
}

generic_container::iterator& generic_container::iterator::operator++()
{
	current_element++;
	return *this;
}

generic_container::iterator& generic_container::iterator::operator=(const iterator& other)
{
	current_element = other.current_element;
	new (&it_container) generic_container(other.it_container);

	return *this;
}

generic_container::element_rep generic_container::iterator::operator*() const
{
	return it_container.get_element_at(current_element);
}

generic_container::element_rep generic_container::iterator::operator->() const
{
	return it_container.get_element_at(current_element);
}

bool generic_container::iterator::operator==(const iterator& other) const
{
	return it_container.layout_hash == other.it_container.layout_hash && other.current_element == current_element;
}

bool generic_container::iterator::operator!=(const iterator& other) const
{
	return other.current_element != current_element || it_container.layout_hash != other.it_container.layout_hash;
}

generic_container::iterator generic_container::end()
{
	return { *this, element_stack_index };
}

generic_container::iterator generic_container::begin()
{
	return  { *this, 0 };
}

void halt()
{
	std::cout << "Halt triggered" << std::endl;
	__debugbreak();
}
