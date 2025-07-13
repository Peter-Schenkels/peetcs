#include "include/app/app.hpp"

#include "include/pipo/rasterizer.hpp"
#include "tests/shared.hpp"


void app::tick_entity_creation()
{
}

void app::tick_component_creation()
{
}

void app::save_level()
{
}

void app::save_entity_type()
{
}

void app::save_entity_type_instance()
{
}




template<typename T>
static void load_component(const json& component_json, peetcs::archetype_pool& pool, const app::entity_type_id target, const pipo& gpu_context)
{
	T& data = pool.add<T>(target);

	if constexpr (requires {T::load_component(component_json, data, gpu_context);})
	{
		T::load_component(component_json, data, gpu_context);
	}

	pool.emplace_commands();
}

void app::load_resources(json& json, pipo& gpu_context)
{
	for (auto& object : json)
	{
		if (!object.contains(format::type_key))
		{
			assert(false);
			continue;
		}

		std::string_view type = object[format::type_key];
		if (type == format::texture_type_key)
		{
			pipo::texture::load_settings settings;
			from_json(object,
				"file_path", settings.file_path,
				"nearest_neighbour", settings.nearest_neighbour,
				"clamp_borders", settings.clamp_borders);

			gpu_context.load_texture_gpu(settings);
		}
	}
}

void app::load_entity_types(const json& json, pipo& gpu_context)
{
	for (const auto& object : json)
	{
		if (!object.contains(format::entity_type_id_key))
		{
			assert(false);
			continue;
		}

		entity_type_id id = object[format::entity_type_id_key];

		if (!object.contains(format::components_key))
		{
			assert(false);
			continue;
		}

		for (auto& component : object[format::components_key])
		{
			if (!component.contains(format::component_id_key))
			{
				assert(false);
				continue;
			}

			switch (component[format::component_id_key].get<uint16_t>())
			{
				case pipo::transform_data::id:                    load_component<pipo::transform_data>(component, entity_types, id, gpu_context);            break;
				case pipo::camera_data::id:                       load_component<pipo::camera_data>(component, entity_types, id, gpu_context);				break;
				case pipo::material_data::id:                     load_component<pipo::material_data>(component, entity_types, id, gpu_context);				break;
				case pipo::mesh_renderer_data::id:                load_component<pipo::mesh_renderer_data>(component, entity_types, id, gpu_context);		break;
				case pipo::lit_material_data::id:                 load_component<pipo::lit_material_data>(component, entity_types, id, gpu_context);			break;
				case pipo::unlit_material_data::id:               load_component<pipo::unlit_material_data>(component, entity_types, id, gpu_context);		break;
				case phesycs_impl::rigid_body_data::id:           load_component<phesycs_impl::rigid_body_data>(component, entity_types, id, gpu_context);	break;
				case phesycs_impl::box_collider_data::id:         load_component<phesycs_impl::box_collider_data>(component, entity_types, id, gpu_context);	break;
				default: break;
			}
		}
	}
}

auto app::instantiate_level() -> void
{
}

void app::instantiate_entity_type()
{
}

void app::instantiate_entity_type_instance()
{
}
