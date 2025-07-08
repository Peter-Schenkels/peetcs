#include <chrono>
#define GLEW_STATIC
#include <GL/glew.h>

#include "include/archetype_pool.hpp"
#include "include/phesycs/phesycs.hpp"

#include <math.h>
#include <numbers>

#include "include/pipo/imgui.hpp"
#include "include/pipo/rasterizer.hpp"
#include "tests/shared.hpp"

struct platform_data
{
	constexpr static uint16_t id = 123;
};

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
			int amount = 300;
			peetcs::entity_id last_entity = 0;
			// Setup car entities
			for (int i = triangle; i < amount + start_entity; i++)
			{
				triangle = i + 1;
				int dim = pow(amount, 1.f / 3.f);
				int x = i % dim - dim/2.f;
				int y = (i / dim) % dim - dim / 2.f;
				int z = i / (dim * dim);
				float padding = 1.f;
				//z = z * padding + z;
				y = y * padding + y;
				x = x * padding + x;
				z = z * padding + z;

				pipo::mesh_renderer_data& mesh_render = pool.add<pipo::mesh_renderer_data>(triangle);
				//mesh_render.mesh_id = rasterizer.get_quad();
				mesh_render.mesh_id = rasterizer.get_cube();
				mesh_render.visible = true;

				pipo::unlit_material_data& material = pool.add<pipo::unlit_material_data>(triangle);
				material.main_texture = cube_texture;

				pipo::transform_data& triangle_transform = pool.add<pipo::transform_data>(triangle);

				phesycs_impl::box_collider_data& box_collider = pool.add<phesycs_impl::box_collider_data>(triangle);
				box_collider.transform.set_scale(1.0, 1.0, 1.0);
				auto& rigidbody = pool.add<phesycs_impl::rigid_body_data>(triangle);
				rigidbody.angular_velocity[0] = 0;
				rigidbody.angular_velocity[1] = 0;
				triangle_transform.set_pos(x, -2 + z, y);
				//rigidbody.set_translational_velocity(-glm::normalize(triangle_transform.get_pos()) * 2.f);

				rigidbody.set_mass(100, box_collider);
				rigidbody.is_static = false;

				float scale = 0.5;
				triangle_transform.set_scale(scale, scale, scale);

				pool.emplace_commands();
			}

			// Setup camera entity
			{
				pipo::camera_data& camera_data = pool.add<pipo::camera_data>(camera);
				pipo::transform_data& camera_transform = pool.add<pipo::transform_data>(camera);
				camera_transform.set_pos(0, 10,  10);
				camera_transform.set_rotation(-3.1415 / 4, 0, 0);

				camera_data.c_near = 0.1f;
				camera_data.c_far = 1000.0f;
				camera_data.aspect = 1920.f / 1080;
				camera_data.fov = 40.0f;
				camera_data.type = pipo::view_type::perspective;
				camera_data.active = true;
				camera_data.render_target = main_render_target;

				pool.emplace_commands();
			}
		}




		for (int i = 1; i < 6; i++)
		{
			peetcs::entity_id platform = triangle + i;
			pipo::mesh_renderer_data& platform_mesh_renderer = pool.add<pipo::mesh_renderer_data>(platform);

			platform_mesh_renderer.visible = true;
			pipo::transform_data& platform_transform = pool.add<pipo::transform_data>(platform);
			pipo::unlit_material_data& material = pool.add<pipo::unlit_material_data>(platform);
			material.main_texture = background_texture;


			phesycs_impl::rigid_body_data& platform_rigidbody = pool.add<phesycs_impl::rigid_body_data>(platform);
			platform_rigidbody.is_static = true;

			phesycs_impl::box_collider_data& platform_collider = pool.add<phesycs_impl::box_collider_data>(platform);

			float scale = 1.f;
			platform_collider.transform.set_scale(scale, scale, scale);

			if (i == 1)
			{
				platform_mesh_renderer.mesh_id = rasterizer.get_plane();
				platform_transform.set_scale(10, 10, 10);
				platform_transform.set_pos(0, -15, -5);
				platform_transform.set_rotation(0, 0, 0);
			}
			else if (i == 2)
			{
				platform_mesh_renderer.mesh_id = rasterizer.get_cube();
				platform_transform.set_scale(1, 2, 10);
				platform_transform.set_pos(10, -3, -5);
				platform_transform.set_rotation(0, 0, 0);


				//pool.add<platform_data>(platform);
				platform_rigidbody.is_static = true;
				//platform_rigidbody.set_mass(100000.f, platform_collider);
				continue;;

			}
			else if (i == 3)
			{
				platform_mesh_renderer.mesh_id = rasterizer.get_cube();
				platform_transform.set_scale(1, 2, 10);
				platform_transform.set_pos(-10, -3, -5);
				platform_transform.set_rotation(0, 0, 0);
				pool.add<platform_data>(platform);
				platform_rigidbody.is_static = true;
				continue;;
			}
			else if (i == 4)
			{
				platform_mesh_renderer.mesh_id = rasterizer.get_cube();
				platform_transform.set_scale(5, 2, 2);
				platform_transform.set_pos(0, -3, 5);
				platform_transform.set_rotation(0, 0, 0);

				pool.add<platform_data>(platform);
				platform_rigidbody.is_static = true;
				//platform_rigidbody.set_mass(100000.f, platform_collider);

			}
			else if (i == 5)
			{
				platform_mesh_renderer.mesh_id = rasterizer.get_cube();
				platform_transform.set_scale(10, 2, 1);
				platform_transform.set_pos(0, -3, -15);
				platform_transform.set_rotation(0, 0, 0);
				//platform_rigidbody.is_static = false;
				//platform_rigidbody.set_mass(100000.f, platform_collider);
				continue;;

			}



			pool.emplace_commands();
		}
	}

	// Setup Debug imguis
	std::vector<std::shared_ptr<gui_interface>> debug_guis = {
		std::make_unique<performance_profiler>(),
		std::make_unique<dll_reloader<phesycs>>()
	};

	time_info time;


	// Do game ticks
	while (rasterizer.start_frame())
	{

		rasterizer.render_imgui(pool, debug_guis);
		rasterizer.render_frame(pool);
		time.tick();

		auto platform_query = pool.query<platform_data, pipo::transform_data>();
		for (auto query_value : platform_query)
		{
			auto& transform = query_value.get<pipo::transform_data>();

			glm::quat rotation = transform.get_rotation();

			transform.set_rotation(glm::angleAxis((float)fmod(glm::angle(rotation) +  time.get_delta_time() * 0.3, std::numbers::pi), glm::vec3{0.f,-1.f,0.f } ));
		}

		if (phesycs::loaded)
		{
			phesycs::tick(pool, rasterizer);
		}
	}

	// Exit
	rasterizer.deinit();

	return 0;
}
