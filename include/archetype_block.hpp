#pragma once
#include <unordered_map>

#include "archetype_id.hpp"
#include "peetcs_types.hpp"
#include "storage.hpp"


namespace peetcs
{
	class archetype_block
	{
		storage block;
		archetype_id archetype_info;

	public:

		archetype_block() = default;

		explicit archetype_block(const archetype_id& id);

		storage::region add_entity(const entity_id entity);

		storage::region get_entity(const entity_id entity);

		void remove_entity(const entity_id entity);

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


			explicit iterator(const storage::iterator& it);

			iterator& operator++();

			storage::iterator operator*() const;
			storage::iterator operator->() const;

			bool operator==(const iterator& other) const;
			bool operator!=(const iterator& other) const;
		};

		iterator begin();

		iterator end();
	};
}