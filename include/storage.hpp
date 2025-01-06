#pragma once
#include <vector>

namespace peetcs
{
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

		storage();
		storage(std::size_t element_size, std::size_t number_of_elements);

		region add_element();

		region get_element(const std::size_t index);

		void remove_element(std::size_t index);

		[[nodiscard]] std::size_t size() const;

		[[nodiscard]] std::size_t last() const;


		class iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = region;
			using difference_type = std::ptrdiff_t;
			using pointer = region*;
			using reference = region&;

			std::size_t current_element = 0;
			storage& it_storage;

			iterator(const iterator& it);

			iterator(storage& storage, const region& start);

			iterator& operator++();

			iterator& operator=(const iterator& other);

			region operator*() const;

			region operator->() const;


			bool operator==(const iterator& other) const;

			// Inequality comparison
			bool operator!=(const iterator& other) const;
		};

		iterator begin();
		iterator end();
	};
}