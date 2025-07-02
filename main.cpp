#include <chrono>
#define GLEW_STATIC
#include <GL/glew.h>

#include "include/archetype_pool.hpp"
#include "include/phesycs/phesycs.hpp"

#include "include/pipo/imgui.hpp"
#include "include/pipo/rasterizer.hpp"
#include "tests/shared.hpp"


int main()
{
	peetcs::archetype_pool pool;

	pipo rasterizer = {};
	rasterizer.init();
	rasterizer.create_window(1920, 1080, "Hello World");

	// Instantiate the scene
	{
		// Setup GPU resources
		rasterizer.init_default_resources();

		pipo::texture_id carkel_texture;
		{
			pipo::texture::load_settings texture_settings;
			char texture_filepath[] = R"(Assets\textures\carkel texture.png)";
			texture_settings.file_path = texture_filepath;
			carkel_texture = rasterizer.load_texture_gpu(texture_settings);
		}

		pipo::texture_id background_texture;
		{
			pipo::texture::load_settings texture_settings;
			char texture_filepath[] = R"(Assets\textures\ground.png)";
			texture_settings.file_path = texture_filepath;
			texture_settings.nearest_neighbour = true;
			background_texture = rasterizer.load_texture_gpu(texture_settings);
		}

		pipo::texture_id cube_texture;
		{
			pipo::texture::load_settings texture_settings;
			texture_settings.nearest_neighbour = true;
			char texture_filepath[] = R"(Assets\textures\cube.png)";
			texture_settings.file_path = texture_filepath;
			cube_texture = rasterizer.load_texture_gpu(texture_settings);
		}

		std::vector<pipo::mesh_id> carkel_mesh;
		{
			pipo::mesh::load_settings mesh_settings;
			char mesh_filepath[] = R"(Assets\models\carkel.obj)";
			mesh_settings.file_path = mesh_filepath;
			rasterizer.load_mesh_gpu(mesh_settings, carkel_mesh);
		}

		pipo::render_target::allocate_settings render_target_settings = {};
		render_target_settings.width = 1920;
		render_target_settings.height = 1080;

		pipo::render_target_id main_render_target = rasterizer.create_render_target(render_target_settings);

		peetcs::entity_id render_target = 0;
		{
			pipo::render_target_renderer_data& target_renderer = pool.add<pipo::render_target_renderer_data>(render_target);
			target_renderer.width = 1920;
			target_renderer.height = 1080;
			target_renderer.x = 0;
			target_renderer.y = 0;
			target_renderer.target_id = main_render_target;
			target_renderer.visible = true;

			pool.emplace_commands();
		}

		// Setup entities
		peetcs::entity_id triangle = 2;
		peetcs::entity_id camera = 1;

		{
			int start_entity = triangle;
			int amount = 100;
			peetcs::entity_id last_entity = 0;
			// Setup car entities
			for (int i = triangle; i < amount + start_entity; i++)
			{
				triangle = i + 1;
				int dim = pow(amount, 1.f / 3.f);
				float z = i / (dim * dim);
				float y = (i % (dim * dim)) / dim - dim /2.f;
				float x = i % dim - dim / 2.f;
				float padding = 1.f;
				z = z * padding + z;
				y = y * padding + y;
				x = x * padding + x;

				pipo::mesh_renderer_data& mesh_render = pool.add<pipo::mesh_renderer_data>(triangle);
				//mesh_render.mesh_id = rasterizer.get_quad();
				mesh_render.mesh_id = rasterizer.get_cube();
				mesh_render.visible = i % 1 == 0;

				pipo::unlit_material_data& material = pool.add<pipo::unlit_material_data>(triangle);
				material.main_texture = cube_texture;

				pipo::transform_data& triangle_transform = pool.add<pipo::transform_data>(triangle);
				//triangle_transform.set_rotation(x/1, x / 1, x / 1);

				phesycs_impl::box_collider_data& box_collider = pool.add<phesycs_impl::box_collider_data>(triangle);
				box_collider.transform.set_scale(1.0, 1.0, 1.0);
				auto& rigidbody = pool.add<phesycs_impl::rigid_body_data>(triangle);
				rigidbody.angular_velocity[0] = 0;
				rigidbody.angular_velocity[1] = 0;
				//rigidbody.set_translational_velocity(-1.f * glm::normalize(triangle_transform.get_pos()));
				triangle_transform.set_pos(x, -3 + z, y);
				//rigidbody.velocity[1] = x * -1;
				rigidbody.set_mass(1, box_collider);
				rigidbody.is_static = false;

				float scale = 0.5;
				triangle_transform.set_scale(scale, scale, scale);

				pool.emplace_commands();
			}

			// Setup camera entity
			{
				pipo::camera_data& camera_data = pool.add<pipo::camera_data>(camera);
				pipo::transform_data& camera_transform = pool.add<pipo::transform_data>(camera);
				//phesycs_impl::rigid_body_data& camera_rigidbody = pool.add<phesycs_impl::rigid_body_data>(camera);

				//phesycs_impl::box_collider_data& camera_collider = pool.add<phesycs_impl::box_collider_data>(camera);

				//camera_rigidbody.set_mass(100.f, camera_collider);
				//camera_rigidbody.is_static = true;
				camera_transform.set_pos(0, 0, 10);
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

		peetcs::entity_id platform = triangle + 1;
		pipo::mesh_renderer_data& platform_mesh_renderer = pool.add<pipo::mesh_renderer_data>(platform);
		platform_mesh_renderer.mesh_id = rasterizer.get_plane();
		platform_mesh_renderer.visible = true;
		pipo::transform_data& platform_transform = pool.add<pipo::transform_data>(platform);
		pipo::unlit_material_data& material = pool.add<pipo::unlit_material_data>(platform);
		material.main_texture = background_texture;

		platform_transform.set_scale(10, 2, 10);
		platform_transform.set_pos(0, -6, -5);
		platform_transform.set_rotation(0, 0, 0);

		phesycs_impl::rigid_body_data& platform_rigidbody = pool.add<phesycs_impl::rigid_body_data>(platform);
		platform_rigidbody.is_static = true;
		phesycs_impl::box_collider_data& platform_collider = pool.add<phesycs_impl::box_collider_data>(platform);

		float scale = 1.f;
		platform_collider.transform.set_scale(scale, scale, scale);
		pool.emplace_commands();
	}

	// Setup Debug imguis
	std::vector<std::shared_ptr<gui_interface>> debug_guis = {
		std::make_unique<performance_profiler>(),
		std::make_unique<dll_reloader<phesycs>>()
	};

	// Do game ticks
	while (rasterizer.start_frame())
	{
		rasterizer.render_imgui(pool, debug_guis);
		rasterizer.render_frame(pool);

		if (phesycs::loaded)
		{
			phesycs::tick(pool, rasterizer);
		}

		rasterizer.draw_cube_gizmo({ 0,4,-10 }, { 1,1,1 }, { 0,0,0,1 }, {0,1,0});
	}

	// Exit
	rasterizer.deinit();

	return 0;
}
