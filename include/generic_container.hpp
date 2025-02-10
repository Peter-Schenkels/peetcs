#pragma once
#include <cstdint>
#include <vcruntime_typeinfo.h>
#include <vector>


void halt();


#define HASH_VALUE_TYPE uint16_t

template <class C>
struct type_id {
	constexpr static int _id{};
	constexpr static HASH_VALUE_TYPE id() {
		return reinterpret_cast<HASH_VALUE_TYPE>(&_id);
	}
};

struct element_layout
{
	using hash_t           = HASH_VALUE_TYPE;
	using stride_t         = uint16_t;
	using id_t             = uint32_t;
	using index_t          = int32_t;
	using element_stride_t = uint32_t;

	struct element_info
	{
		id_t             element_id = std::numeric_limits<id_t>::max();
		index_t          index      = std::numeric_limits<index_t>::max();
		element_stride_t stride     = std::numeric_limits<element_stride_t>::max();
		bool             operator==(const element_info& element_info) const;
	};

	struct sub_element_info
	{
		hash_t id;
		stride_t stride;
	};

	struct type_info
	{
		hash_t id;
		stride_t stride;


		type_info(hash_t id, stride_t stride):
			id(id),
			stride(stride)
		{}

		template<typename T>
		static type_info init()
		{
			return type_info { type_id<T>::id(), sizeof(T)};
		}
	};

	// generic container buffer looks like this:
	// (start meta info)
	// [type][stride]
	// [type][stride]
	// (Start elements)
	// [id][stride]
	//  [type][stride][sub-element]
	//  [type][stride][sub-element]
	// [id][stride]
	//  [type][stride][sub-element]
	//  [type][stride][sub-element]
	std::vector<type_info> layout = {};
	std::size_t element_size = sizeof(element_info);
	std::size_t meta_size = 0;
	std::size_t cached_hash = std::size_t{std::numeric_limits<std::size_t>::max()};

	void add(const hash_t& id, const stride_t& stride);

	template <typename T>
	void add()
	{
		add(type_id<T>::id(), sizeof(T));
	}

	bool contains(const hash_t& type) const;
	void remove(hash_t type_index);

	std::size_t hash();

	static void* get_sub_element(hash_t type, void* element_ptr);

	bool operator==(const element_layout& other) const
	{
		if (cached_hash == std::numeric_limits<std::size_t>::max() || other.cached_hash == std::numeric_limits<std::size_t>::max())
		{
			halt();
		}
		return cached_hash == other.cached_hash;
	}
};

template <>
struct std::hash<element_layout>
{
	std::size_t operator()(const element_layout& k) const noexcept
	{
		if (k.cached_hash == std::numeric_limits<std::size_t>::max())
		{
			halt();
		}

		return k.cached_hash;
	}
};


struct generic_container
{
	class element_rep
	{
		friend generic_container;

		generic_container& parent_container;
		element_layout::element_info value_at_pointer;
		void* element_ptr = nullptr;

		element_rep(void* element_ptr, generic_container& parent);

		void verify_ptr();

	public:
		[[nodiscard]] element_layout::id_t get_id() const;

		[[nodiscard]] element_layout::index_t get_index() const;

		template<typename T>
		void set(const T& new_data)
		{
			set_generic_ptr(type_id<T>::id(), sizeof(T), &new_data);
		}

		void set_generic_ptr(const element_layout::hash_t type, const std::size_t stride, const void* new_data);

		template<typename T>
		T& get()
		{
			verify_ptr();

			T* sub_element_ptr = static_cast<T*>(element_layout::get_sub_element(type_id<T>::id(), static_cast<uint8_t*>(element_ptr)));
			return *sub_element_ptr;
		}

		template<typename T>
		T* get_ptr()
		{
			verify_ptr();

			T* sub_element_ptr = static_cast<T*>(element_layout::get_sub_element(type_id<T>::id(), static_cast<uint8_t*>(element_ptr)));
			return sub_element_ptr;
		}

		void* get_ptr_generic(const element_layout::hash_t type);
		void copy_sub_elements(const element_rep& old_region) const;
	};

	std::vector<uint8_t> data;
	element_layout layout;
	std::size_t layout_hash;
	element_layout::index_t max = 0;
	element_layout::index_t element_stack_index = 0;

	generic_container() = default;

#ifndef DEPRECATED
	generic_container(const std::vector<element_layout::type_info>& types, element_layout::index_t nb_of_elements);
#endif

	generic_container(element_layout in_layout, element_layout::index_t nb_of_elements);

	[[nodiscard]] std::size_t get_byte_index(const element_layout::index_t element_index) const;

	element_rep add_element(const element_layout::id_t id);

	element_rep get_element_at(const element_layout::index_t index);

	element_rep get_element(const element_layout::id_t id);

	bool contains(const element_layout::id_t id);

	void remove_at(const element_layout::index_t index);

	void pop();

	void remove(const element_layout::id_t id);

	inline std::size_t size() const { return element_stack_index; }

	void resize(std::size_t new_size);

	element_rep last() { return get_element_at(element_stack_index - 1); }

	class iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = element_rep;
		using difference_type = std::ptrdiff_t;
		using pointer = element_rep*;
		using reference = element_rep&;

		element_layout::index_t current_element = 0;
		generic_container& it_container;

		iterator(const iterator& it);

		iterator(generic_container& container, element_layout::index_t start);

		iterator& operator++();

		element_rep operator*() const;

		element_rep operator->() const;

		bool operator==(const iterator& other) const;

		bool operator!=(const iterator& other) const;
	};

	iterator end();

	iterator begin();
};

