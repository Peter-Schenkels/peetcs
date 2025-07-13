#pragma once
#include "include/archetype_pool.hpp"
#include "include/utilities.hpp"


class pipo;

struct app
{
	struct format
	{
		constexpr static std::string_view entity_type_id_key = "entity_type_id";
		constexpr static std::string_view texture_type_key = "texture";
		constexpr static std::string_view type_key = "type";
		constexpr static std::string_view components_key = "components";
		constexpr static std::string_view component_id_key = "component_id";
	};


	using entity_type_id = int;
	using entity_type_instance_id = int;
	using level_id = int;

	struct create_entity_data
	{
		
	};

	struct create_component_data
	{
		
	};

	peetcs::archetype_pool entity_types;

	// System functions
	void tick_entity_creation();
	void tick_component_creation();

	// API functions
	void save_level();
	void save_entity_type();
	void save_entity_type_instance();

	void load_resources(json& json, pipo& gpu_context);
	void load_entity_types(const json& json, pipo& gpu_context);

	void instantiate_level();
	void instantiate_entity_type();
	void instantiate_entity_type_instance();
};
