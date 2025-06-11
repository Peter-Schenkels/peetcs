#include <chrono>
#define GLEW_STATIC
#include <GL/glew.h>

#include "include/archetype_pool.hpp"

#include "include/pipo/imgui.hpp"
#include "include/pipo/rasterizer.hpp"
#include "tests/shared.hpp"


int main()
{
	peetcs::archetype_pool pool;

	pipo::init();
	pipo::create_window(1920, 1080, "Hello World");

	// Instantiate the scene
	{
		// Setup GPU resources
		pipo::resources::init_default_resources();

		pipo::mesh::load_settings mesh_settings;
		char mesh_filepath[] = R"(K:\Users\Peter Werk\Documents\GitHub\peetcs\Assets\models\carkel.obj)";
		mesh_settings.file_path = mesh_filepath;

		pipo::texture::load_settings texture_settings;
		char texture_filepath[] = R"(K:\Users\Peter Werk\Documents\GitHub\peetcs\Assets\textures\carkel texture.png)";
		texture_settings.file_path = texture_filepath;
		pipo::texture_id carkel_texture = pipo::resources::load_texture_gpu(texture_settings);

		std::vector<pipo::mesh_id> meshes;
		pipo::resources::load_mesh_gpu(mesh_settings, meshes);

		pipo::render_target::allocate_settings render_target_settings = {};
		render_target_settings.width = 250.f * (1920.f / 1080);
		render_target_settings.height = 250.f;

		pipo::render_target_id main_render_target = pipo::resources::create_render_target(render_target_settings);

		peetcs::entity_id render_target = 2;
		{
			pipo::render_target_renderer_data& target_renderer = pool.add<pipo::render_target_renderer_data>(render_target);
			target_renderer.width = 250;
			target_renderer.height = 250;
			target_renderer.x = 500;
			target_renderer.y = 400;
			target_renderer.target_id = main_render_target;
			target_renderer.visible = true;

			pool.emplace_commands();
		}

		// Setup entities
		peetcs::entity_id triangle = 1;
		peetcs::entity_id camera = 0;

		{
			int amount = 1000;

			// Setup car entities
			for (int i = 0; i < amount; i++)
			{
				triangle = i + 1;

				int x = i % (int)sqrt(amount) - sqrt(amount) / 2;
				int y = i / (int)sqrt(amount) - sqrt(amount) / 2;

				pipo::mesh_renderer_data& mesh_render = pool.add<pipo::mesh_renderer_data>(triangle);
				mesh_render.mesh_id = meshes.front();
				mesh_render.visible = i % 1 == 0;

				pipo::unlit_material_data& material = pool.add<pipo::unlit_material_data>(triangle);
				material.main_texture = carkel_texture;

				pipo::transform_data& triangle_transform = pool.add<pipo::transform_data>(triangle);
				triangle_transform.set_pos(x, y, 0);
				triangle_transform.set_rotation(x, y, 0);
				triangle_transform.set_scale(0.1f, 0.1f, 0.1f);

				pool.emplace_commands();
			}

			// Setup camera entity
			{
				pipo::camera_data& camera_data = pool.add<pipo::camera_data>(camera);
				pipo::transform_data& camera_transform = pool.add<pipo::transform_data>(camera);

				camera_transform.set_pos(0, 0, 2);
				camera_transform.set_rotation(0, 0, 0);

				camera_data.c_near = 0.1f;
				camera_data.c_far = 1000.0f;
				camera_data.aspect = 1920.f / 1080;
				camera_data.fov = 60.0f;
				camera_data.type = pipo::view_type::perspective;
				camera_data.active = true;
				camera_data.render_target = main_render_target;

				pool.emplace_commands();
			}
		}
	}

	// Setup Debug imguis
	std::vector<std::shared_ptr<gui_interface>> debug_guis = {
		std::make_unique<performance_profiler>(),
		std::make_unique<dll_reloader<phesycs>>()
	};

	// Do game ticks
	while (pipo::start_frame())
	{
		pipo::render_imgui(pool, debug_guis);
		pipo::render_frame(pool);

		auto camera_query = pool.query<pipo::camera_data, pipo::transform_data>();
		for (auto camera_value : camera_query)
		{
			pipo::transform_data& camera_transform = camera_value.get<pipo::transform_data>();
			camera_transform.position[0] += 0.001f;
			camera_transform.position[1] += 0.001f;
			camera_transform.position[2] += 0.010f;
			camera_transform.rotation[2] -= 0.001f;
		}

		auto mesh_query = pool.query<pipo::mesh_renderer_data, pipo::transform_data>();
		for (auto query : mesh_query)
		{
			pipo::transform_data& transform = query.get<pipo::transform_data>();
			transform.rotation[2] += 0.1f;
		}

		auto render_target_query = pool.query<pipo::render_target_renderer_data>();
		for (auto query : render_target_query)
		{
			pipo::render_target_renderer_data& render_target = query.get<pipo::render_target_renderer_data>();
			render_target.x += 1;
			render_target.width += 2;
		}

		if (phesycs::loaded)
		{
			phesycs::tick(pool);
		}
	}

	// Exit
	pipo::deinit();

	return 0;
}
