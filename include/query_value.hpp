#pragma once
#include "archetype_id.hpp"

namespace peetcs
{
	struct query_value
	{
		archetype_id& memory_accesor;
		storage::region region;

		query_value(archetype_id& memory_accesor, const storage::region& region);

template<typename Component>
Component& get()
{
    // Precompute values to avoid repeated calculation
    auto* storage_ptr = static_cast<storage::storage_type*>(region.storage_start);
    const std::size_t nb_of_types_in_element = *storage_ptr;
    const std::size_t component_hash = typeid(Component).hash_code();
    const std::size_t base_offset = storage::padding_nb_of_element + nb_of_types_in_element * storage::padding_element_type;

    std::size_t element_offset = 0;
    const std::size_t hash_mask = 0xFFFFFFF8;  // Mask to compare hashes

    // Start searching the region for the component
    for (size_t offset = 1; offset < (base_offset + nb_of_types_in_element * storage::padding_element_type);)
    {
        // Read the first byte of the hash and compare it
        const uint8_t retrieved_hash_1 = storage_ptr[offset];

        if ((retrieved_hash_1 & hash_mask) == (component_hash & hash_mask)) [[likely]]
        {
            // We found a matching hash, now read the next 3 bytes and check the full hash
            const uint64_t retrieved_hash = *reinterpret_cast<const uint64_t*>(&storage_ptr[offset + 1]);

            if (retrieved_hash == component_hash) [[likely]]
            {
                // If hashes match, calculate the component offset
                break;
            }
        }

        // Skip to the next element's header
        offset += 4;
        const uint8_t element_size_1 = storage_ptr[offset];
        const uint8_t element_size_2 = storage_ptr[offset + 1];

        element_offset += (element_size_1 << 1) | element_size_2;  // Combining the size into a single offset increment
    }

    // Get the component pointer based on the calculated offset
    Component* component_ptr = reinterpret_cast<Component*>(static_cast<storage::storage_type*>(region.data) + element_offset);
    return *component_ptr;
}
	};
}