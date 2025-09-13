#define GLEW_STATIC
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "include/archetype_pool.hpp"
#include "include/phesycs/phesycs.hpp"

#include <math.h>
#include <numbers>
#include <regex>

#include "include/pipo/imgui.hpp"
#include "include/pipo/rasterizer.hpp"
#include "tests/shared.hpp"

struct platform_data
{
	constexpr static uint16_t id = 123;
};

struct claw_data
{
	constexpr static uint16_t id = 124;
};


using node_id = size_t;

int main()
{
	peetcs::archetype_pool pool;

	float height = 1500;
	float width = 1080;

	pipo rasterizer = {};
	rasterizer.init();
	rasterizer.create_window(width, height,  "Claw Machine - Physics Demo");

	// Instantiate the scene

	// Setup GPU resources
	rasterizer.init_default_resources();

	texture_id carkel_texture;
	{
		pipo::texture::load_settings texture_settings;
		char texture_filepath[] = R"(Assets\textures\carkel texture.png)";
		texture_settings.file_path = texture_filepath;
		carkel_texture = rasterizer.load_texture_gpu(texture_settings);
	}

	texture_id background_texture;
	{
		pipo::texture::load_settings texture_settings;
		char texture_filepath[] = R"(Assets\textures\ground.png)";
		texture_settings.file_path = texture_filepath;
		texture_settings.nearest_neighbour = true;
		background_texture = rasterizer.load_texture_gpu(texture_settings);
	}

	texture_id cube_texture;
	{
		pipo::texture::load_settings texture_settings;
		texture_settings.nearest_neighbour = true;
		char texture_filepath[] = R"(Assets\textures\cube.png)";
		texture_settings.file_path = texture_filepath;
		cube_texture = rasterizer.load_texture_gpu(texture_settings);
	}

	texture_id cable_texture;
	{
		pipo::texture::load_settings texture_settings;
		texture_settings.nearest_neighbour = true;
		char texture_filepath[] = R"(Assets\textures\cable.png)";
		texture_settings.file_path = texture_filepath;
		cable_texture = rasterizer.load_texture_gpu(texture_settings);
	}

	texture_id claw_texture;
	{
		pipo::texture::load_settings texture_settings;
		texture_settings.nearest_neighbour = true;
		char texture_filepath[] = R"(Assets\textures\claw.png)";
		texture_settings.file_path = texture_filepath;
		claw_texture = rasterizer.load_texture_gpu(texture_settings);
	}

	texture_id claw_machine_texture;
	{
		pipo::texture::load_settings texture_settings;
		texture_settings.nearest_neighbour = true;
		char texture_filepath[] = R"(Assets\textures\crane game.png)";
		texture_settings.file_path = texture_filepath;
		claw_machine_texture = rasterizer.load_texture_gpu(texture_settings);
	}

	texture_id radio_texture;
	{
		pipo::texture::load_settings texture_settings;
		texture_settings.nearest_neighbour = true;
		char texture_filepath[] = R"(Assets\textures\radio.png)";
		texture_settings.file_path = texture_filepath;
		radio_texture = rasterizer.load_texture_gpu(texture_settings);
	}

	std::vector<mesh_id> radio_mesh;
	{
		pipo::mesh::load_settings mesh_settings;
		char mesh_filepath[] = R"(Assets\models\radio.obj)";
		mesh_settings.file_path = mesh_filepath;
		rasterizer.load_mesh_gpu(mesh_settings, radio_mesh);
	}


	std::vector<mesh_id> carkel_mesh;
	{
		pipo::mesh::load_settings mesh_settings;
		char mesh_filepath[] = R"(Assets\models\carkel.obj)";
		mesh_settings.file_path = mesh_filepath;
		rasterizer.load_mesh_gpu(mesh_settings, carkel_mesh);
	}

	std::vector<mesh_id> claw_mesh;
	{
		pipo::mesh::load_settings mesh_settings;
		char mesh_filepath[] = R"(Assets\models\claw.obj)";
		mesh_settings.file_path = mesh_filepath;
		rasterizer.load_mesh_gpu(mesh_settings, claw_mesh);
	}

	std::vector<mesh_id> claw_machine_mesh;
	{
		pipo::mesh::load_settings mesh_settings;
		char mesh_filepath[] = R"(Assets\models\claw machine.obj)";
		mesh_settings.file_path = mesh_filepath;
		rasterizer.load_mesh_gpu(mesh_settings, claw_machine_mesh);
	}

	std::vector<mesh_id> l_mesh;
	{
		pipo::mesh::load_settings mesh_settings;
		char mesh_filepath[] = R"(Assets\models\l-shape.obj)";
		mesh_settings.file_path = mesh_filepath;
		rasterizer.load_mesh_gpu(mesh_settings, l_mesh);
	}

	pipo::render_target::allocate_settings render_target_settings = {};
	render_target_settings.width = width;
	render_target_settings.height = height;
	render_target_settings.name = "default";

	render_target_id main_render_target = rasterizer.create_render_target(render_target_settings);

	peetcs::entity_id render_target = 0;
	{
		pipo::render_target_renderer_data& target_renderer = pool.add<pipo::render_target_renderer_data>(render_target);
		target_renderer.width = width;
		target_renderer.height = height;
		target_renderer.x = 0;
		target_renderer.y = 0;
		target_renderer.target_id = main_render_target;
		target_renderer.visible = true;

		pool.emplace_commands();
	}

	// Setup entities
	peetcs::entity_id camera = 1;
	{
		for (int i = 0; i < 1; i++)
		// Setup camera entity
		{
			pipo::camera_data& camera_data = pool.add<pipo::camera_data>(camera);
			pipo::transform_data& camera_transform = pool.add<pipo::transform_data>(camera);
			camera_transform.set_pos(-0.5, 20, 30.5);
			camera_transform.set_rotation(-3.1415 / 8, 0, 0);

			camera_data.c_near = 0.1f;
			camera_data.c_far = 1000.0f;
			camera_data.aspect = width / height;
			camera_data.fov = 80.0f;
			camera_data.type = pipo::view_type::perspective;
			camera_data.active = i == 0;
			camera_data.render_target = main_render_target;

			pool.emplace_commands();

			camera++;
		}
	}

	peetcs::entity_id block = camera + 1;

	// Setup block entities
	int i = 0;
	int max = 100;
	for (int x = -3; x < 4; x++)
	{
		for (int y = 0; y < 9; y++)
		{
			for (int z = 0; z <4; z++)
			{
				block = i + 2;

				float padding = 2.f;
				//z = z * padding + z;
				float  y_pos = (float)y * padding + (float)y;
				float x_pos = (float)x * padding + (float)x;
				float z_pos = (float)z * padding + (float)z;

				pipo::mesh_renderer_data& mesh_render = pool.add<pipo::mesh_renderer_data>(block);
				//mesh_render.mesh_id = rasterizer.get_quad();
				mesh_render.mesh = rasterizer.get_cube();
				mesh_render.visible = true;

				pipo::unlit_material_data& material = pool.add<pipo::unlit_material_data>(block);
				material.main_texture = cube_texture;

				pipo::transform_data& triangle_transform = pool.add<pipo::transform_data>(block);
				float scale = 1.f;

				phesycs_impl::box_collider_data& box_collider = pool.add<phesycs_impl::box_collider_data>(block);
				box_collider.transform.set_scale(1.0, 1.0, 1.0);
				box_collider.transform.set_pos(0, 0, 0);

				if (i % 2)
				{
					box_collider.transform.set_pos(0, 0.5, 0);
					box_collider.transform.set_scale(0.5, 0.45, 1.);
					mesh_render.mesh = radio_mesh.front();
					material.main_texture = radio_texture;
					scale = 2.f;
				}

				triangle_transform.set_scale(scale, scale, scale);




				auto& rigidbody = pool.add<phesycs_impl::rigid_body_data>(block);
				//rigidbody.set_angular_velocity({ 0.22 + i,1,1 });
				rigidbody.set_mass(0.1, box_collider);
				triangle_transform.set_pos(x_pos, -2 + y_pos, z_pos);

				pool.emplace_commands();

				i++;


				if (i == max)
				{
					break;
				}
			}

			if (i == max)
			{
				break;
			}
		}

		if (i == max)
		{
			break;
		}
	}

	peetcs::entity_id platform = block;

	peetcs::entity_id x_claw_arm_bar = platform + 1;
	{
		pipo::mesh_renderer_data& claw_arm_bar_renderer = pool.add<pipo::mesh_renderer_data>(x_claw_arm_bar);
		claw_arm_bar_renderer.visible = true;
		claw_arm_bar_renderer.mesh = rasterizer.get_cube();

		pipo::transform_data& claw_arm_bar_transform = pool.add<pipo::transform_data>(x_claw_arm_bar);
		claw_arm_bar_transform.set_scale(1, 1, 10);
		claw_arm_bar_transform.set_pos(-5, 25,8);
		claw_arm_bar_transform.set_rotation(0, 0, 0);

		pipo::unlit_material_data& material = pool.add<pipo::unlit_material_data>(x_claw_arm_bar);
		material.main_texture = background_texture;

		phesycs_impl::box_collider_data& claw_arm_bar_collider = pool.add<phesycs_impl::box_collider_data>(x_claw_arm_bar);
		claw_arm_bar_collider.transform = {};
		float scale = 1.f;
		claw_arm_bar_collider.transform.set_scale(scale, scale, scale);


		phesycs_impl::rigid_body_data& claw_arm_bar_rigidbody = pool.add<phesycs_impl::rigid_body_data>(x_claw_arm_bar);
		claw_arm_bar_rigidbody.is_static = true;
		claw_arm_bar_rigidbody.gravity = false;
		claw_arm_bar_rigidbody.set_mass(100, claw_arm_bar_collider);

		pool.emplace_commands();
	}

	peetcs::entity_id y_claw_arm_bar = x_claw_arm_bar + 1;
	{
		pipo::mesh_renderer_data& claw_arm_bar_renderer = pool.add<pipo::mesh_renderer_data>(y_claw_arm_bar);
		claw_arm_bar_renderer.visible = false;
		claw_arm_bar_renderer.mesh = rasterizer.get_cube();

		pipo::transform_data& claw_arm_bar_transform = pool.add<pipo::transform_data>(y_claw_arm_bar);
		claw_arm_bar_transform.set_scale(1, 1, 0.1);
		claw_arm_bar_transform.set_pos(0, -2, 0);
		claw_arm_bar_transform.set_rotation(0, 0, 0);
		claw_arm_bar_transform.parent = x_claw_arm_bar;

		pipo::unlit_material_data& material = pool.add<pipo::unlit_material_data>(y_claw_arm_bar);
		material.main_texture = cube_texture;


		phesycs_impl::rigid_body_data& claw_arm_bar_rigidbody = pool.add<phesycs_impl::rigid_body_data>(y_claw_arm_bar);
		claw_arm_bar_rigidbody.is_static = true;
		claw_arm_bar_rigidbody.gravity = false;

		pool.emplace_commands();
	}

	peetcs::entity_id claw = 0;
	peetcs::entity_id claw_machine = y_claw_arm_bar + 1;
	{
		auto& transform = pool.add<pipo::transform_data>(claw_machine);
		transform.set_pos(2, -15, 5);
		transform.set_scale(5, 5, 5);
		transform.set_rotation(0, 0, 0);

		// Bug!!!!
		auto& claw_mesh_renderer = pool.add<pipo::mesh_renderer_data>(claw_machine);
		claw_mesh_renderer.visible = true;
		claw_mesh_renderer.mesh = claw_machine_mesh.front();

		auto& material = pool.add<pipo::unlit_material_data>(claw_machine);
		material.main_texture = claw_machine_texture;

		pool.emplace_commands();
	}


	auto transform_data = pool.get_from_owner<pipo::transform_data>(x_claw_arm_bar);
	glm::vec3 claw_pos = transform_data->get_ws_pos(pool);

	float spring_length = 0.1;
	float spring_stiffnes = 800;
	float spring_damping = 10;

	peetcs::entity_id cable = claw_machine;
	int amount_of_chains = 10;
	for (int i = 1; i < amount_of_chains; i++)
	{
		cable = claw_machine + i;


		pipo::transform_data& cable_transform = pool.add<pipo::transform_data>(cable);
		cable_transform.set_pos(claw_pos.x, -i * 1.5 + claw_pos.y, claw_pos.z);
		cable_transform.set_scale(0.1f, 0.1f, 0.1f);
		cable_transform.set_rotation(0, 0, 0);

		phesycs_impl::box_collider_data& cable_collider = pool.add<phesycs_impl::box_collider_data>(cable);
		cable_collider.transform.set_pos(0, 0, 0);
		cable_collider.transform.set_scale(1, 1, 1);
		cable_collider.transform.set_rotation(0, 0, 0);

		phesycs_impl::rigid_body_data& cable_rigidbody = pool.add<phesycs_impl::rigid_body_data>(cable);
		cable_rigidbody.set_mass(1, cable_collider);
		cable_rigidbody.is_static = false;


		phesycs_impl::spring_mass_data& spring_mass = pool.add<phesycs_impl::spring_mass_data>(cable);

		if (i == 1)
		{
			spring_mass.b = y_claw_arm_bar;
		}
		else
		{
			spring_mass.b = cable - 1;
		}
		spring_mass.damping = spring_damping;
		spring_mass.rest_length = spring_length;
		spring_mass.stiffness = spring_stiffnes;

		if (i == amount_of_chains - 2)
		{
			cable_collider.transform.set_scale(0, 0, 0);
		}

		if (i == amount_of_chains - 1)
		{
			claw = cable;

			pipo::mesh_renderer_data& cable_mesh_renderer = pool.add<pipo::mesh_renderer_data>(claw);
			cable_mesh_renderer.visible = true;
			cable_mesh_renderer.mesh = claw_mesh.front();

			pipo::unlit_material_data& material = pool.add<pipo::unlit_material_data>(claw);
			material.main_texture = claw_texture;

			cable_collider.transform.set_scale(1, 3, 1);
			cable_collider.transform.set_pos(0, -2, 0);

			cable_transform.set_scale(1, 1, 1);
			cable_rigidbody.set_mass(100, cable_collider);


			pool.add<claw_data>(claw);
			pool.emplace_commands();


			/*
			auto render_data = pool.get_from_owner<pipo::mesh_renderer_data>(claw);
			render_data->visible = true;
			render_data->mesh = claw_mesh.front();*/

			/*
			material = *pool.get_from_owner<pipo::unlit_material_data>(claw);
			material.main_texture = claw_texture;*/

		}
		else
		{
			pool.emplace_commands();
		}

	}

	peetcs::entity_id colliders = cable + 1;
	for (int i = 0; i < 6; i++)
	{
		pipo::mesh_renderer_data& colliders_mesh_renderer = pool.add<pipo::mesh_renderer_data>(colliders);
		colliders_mesh_renderer.visible = true;
		colliders_mesh_renderer.mesh = rasterizer.get_cube();

		pipo::transform_data& colliders_transform = pool.add<pipo::transform_data>(colliders);
		phesycs_impl::box_collider_data& colliders_collider = pool.add<phesycs_impl::box_collider_data>(colliders);

		pipo::unlit_material_data& material = pool.add<pipo::unlit_material_data>(colliders);
		material.main_texture = background_texture;

		phesycs_impl::rigid_body_data& colliders_rigidbody = pool.add<phesycs_impl::rigid_body_data>(colliders);
		colliders_rigidbody.set_mass(1, colliders_collider);
		colliders_rigidbody.is_static = true;

		pool.add<platform_data>(colliders);

		switch (i)
		{
		case 0: // Collider Back
			colliders_transform.set_pos(-0.5, 1, -4.5);
			colliders_transform.set_scale(12.5, 30, 1);
			colliders_transform.set_rotation(0, 0, 0);
			colliders_mesh_renderer.visible = false;
			break;
		case 1: // Collider left
			colliders_transform.set_pos(-13.60, 4, 0);
			colliders_transform.set_scale(1,30,10);
			colliders_transform.set_rotation(0, 0, 0);
			colliders_mesh_renderer.visible = false;
			break;
		case 2: // Collider Right
			colliders_transform.set_pos(12.650, 4, 0);
			colliders_transform.set_scale(1, 30, 10);
			colliders_transform.set_rotation(0, 0, 0);
			colliders_mesh_renderer.visible = false;
			break;
		case 3: // Collider Front
			colliders_transform.set_pos(-0.5, 1,10.5);
			colliders_transform.set_scale(12.5, 30, 1);
			colliders_transform.set_rotation(0, 0, 0);
			colliders_mesh_renderer.visible = false;
			break;
		case 4: // Collider botton - right / front
			colliders_transform.set_pos(4,-7.9,0);
			colliders_transform.set_scale(8,3,10);
			colliders_transform.set_rotation(0, 0, 0);
			colliders_mesh_renderer.visible = false;
			break;
		case 5: // Collider bottom - left / back
			colliders_transform.set_pos(-5.5,-7.9,-0.9);
			colliders_transform.set_scale(7.5,3,4);
			colliders_transform.set_rotation(0, 0, 0);
			colliders_mesh_renderer.visible = false;
			break;
		}


		colliders += 1;
		pool.emplace_commands();
	}

	// Setup Debug imguis
	std::vector<std::shared_ptr<gui_interface>> debug_guis = {
		std::make_unique<performance_profiler>(),
		std::make_unique<dll_reloader<phesycs>>()
	};

	//phesycs::load_dll();
	time_info time;

	aabb bounds = { {-11,-3,-1}, {10,30,10} };
	// Do game ticks
	while (rasterizer.start_frame())
	{
		rasterizer.start_imgui();
		rasterizer.render_imgui(pool, debug_guis);

		time.tick();


		json json = {};

		// Main static colliders
		auto platform_query = pool.query<platform_data, pipo::transform_data>();
		for (auto query_value : platform_query)
		{
			auto& transform = query_value.get<pipo::transform_data>();

			std::string name_str("Platform: " + std::to_string(query_value.get_id()));
			const char* name = name_str.c_str();

			draw_imgui_editable(name,
			                    "position", transform.position,
			                    "rotation", transform.rotation,
			                    "scale", transform.scale,
			                    "parent", transform.parent);

			transform.dirty = true;
		}

		// Camera controls
		auto camera_query = pool.query<pipo::camera_data, pipo::transform_data>();
		for (auto query_value : camera_query)
		{
			auto name_str = "Camera" + std::string{static_cast<char>(query_value.get_id())};

			ImGui::Begin(name_str.c_str());
			auto& camera = query_value.get<pipo::camera_data>();
			auto& transform = query_value.get<pipo::transform_data>();

			draw_imgui_editable("Transform",
			                    "position", transform.position,
			                    "rotation", transform.rotation,
			                    "scale", transform.scale);

			draw_imgui_editable("Camera",
			                    "active", camera.active,
			                    "fov", camera.fov,
			                    "c_near", camera.c_near,
			                    "c_far", camera.c_far,
			                    "aspect", camera.aspect);
			ImGui::End();
		}

		// Claw machine X/Z controls
		glm::vec3 dir_x = {};
		glm::vec3 dir_y = {};
		if (rasterizer.is_key_pressed(GLFW_KEY_W))
		{
			dir_y += glm::vec3{0, 0, -1};
		}
		if (rasterizer.is_key_pressed(GLFW_KEY_S))
		{
			dir_y += glm::vec3{0, 0, 1};
		}
		if (rasterizer.is_key_pressed(GLFW_KEY_A))
		{
			dir_x += glm::vec3{-1, 0, 0};
		}
		if (rasterizer.is_key_pressed(GLFW_KEY_D))
		{
			dir_x += glm::vec3{1, 0, 0};
		}

		float min[3] = { bounds.min.x, bounds.min.y, bounds.min.z };
		float max[3] = { bounds.max.x, bounds.max.y, bounds.max.z };

		draw_imgui_editable("Bounds",
			"Min", bounds.min,
			"Max", bounds.max);

		rasterizer.draw_aabb(bounds.min, bounds.max, {0, 0, 1});

		if (length(dir_x) > 0)
		{
			float speed = 5;
			dir_x = normalize(dir_x);
			auto* platform_transform = pool.get_from_owner<pipo::transform_data>(x_claw_arm_bar);

			glm::vec3 last_pos = platform_transform->get_pos();
			platform_transform->set_pos(
				platform_transform->get_pos() + dir_x * static_cast<float>(time.get_delta_time()) * speed);

			glm::vec3 current_pos = platform_transform->get_pos();

			if (!bounds.overlap(current_pos))
			{
				platform_transform->set_pos(last_pos);
			}
		}
		if (length(dir_y) > 0)
		{
			float speed = 3.f/10.f;
			dir_y = normalize(dir_y);
			auto* platform_transform = pool.get_from_owner<pipo::transform_data>(y_claw_arm_bar);
			glm::vec3 last_pos = platform_transform->get_pos();
			platform_transform->set_pos(
				platform_transform->get_pos() + dir_y * static_cast<float>(time.get_delta_time()) * speed);
			glm::vec3 current_pos = platform_transform->get_ws_pos(pool);
			if (!bounds.overlap(current_pos))
			{
				platform_transform->set_pos(last_pos);
			}
		}

		// Claw machine Y controls
		float rise = 0;
		if (rasterizer.is_key_pressed(GLFW_KEY_Q))
		{
			rise = 1;
		}
		if (rasterizer.is_key_pressed(GLFW_KEY_E))
		{
			rise = -1;
		}
		if (rise != 0)
		{
			float speed = -5.f;
			auto spring_query = pool.query<phesycs_impl::spring_mass_data>();
			for (auto query_val : spring_query)
			{
				auto& spring_mass = query_val.get<phesycs_impl::spring_mass_data>();
				spring_mass.rest_length += rise * static_cast<float>(time.get_delta_time()) * speed;
			}
		}

		// DLL stuff
		if (phesycs::loaded)
		{
			phesycs::tick(pool, rasterizer);
		}

		pipo::transform_data* claw_transform = pool.get_from_owner<pipo::transform_data>(claw);
		phesycs_impl::box_collider_data* claw_rigidbody = pool.get_from_owner<phesycs_impl::box_collider_data>(claw);
		if (claw_transform && claw_rigidbody)
		{
			draw_imgui_editable("Claw Collider Transform",
				"position", claw_rigidbody->transform.position,
				"rotation", claw_rigidbody->transform.rotation,
				"scale", claw_rigidbody->transform.scale);
			claw_rigidbody->transform.dirty = true;
			
			rasterizer.draw_vertices(std::vector<glm::vec3>(claw_rigidbody->vertices.begin(), claw_rigidbody->vertices.end()), pipo::primitives::cube::indices, { 0, 1, 0 });
		}

		rasterizer.end_imgui();
		rasterizer.render_frame(pool);
	}

	// Exit
	rasterizer.deinit();

	return 0;
}
