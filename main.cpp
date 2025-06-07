#include <iostream>
#include <chrono>
#define GLEW_STATIC
#include <GL/glew.h>

#include "include/archetype_pool.hpp"
#include "tests/unit_tests.hpp"
#include "tests/shared.hpp"
#include <GLFW/glfw3.h>
#include <glm/gtx/quaternion.hpp>

#include "include/pipo/rasterizer.hpp"


struct pingas
{
	float a;
};

void run_performance_simulation_test(const int entity_count)
{
	peetcs::archetype_pool pool;
	for (int i = 0; i < 60; i++)
	{
		std::cout << "Adding components to " << entity_count << " entities";
		int nb_of_components = 0;
		// Measure time for adding components
		auto start = std::chrono::high_resolution_clock::now();

		int counter_3 = 0;
		int counter_5 = 0;
		int counter_7 = 0;
		for (int i = 0; i < entity_count; ++i)
		{
			pool.add<position>(i).x = static_cast<float>(i);
			pool.add<velocity>(i).vx = static_cast<float>(i * 0.1);
			//pool.add<pingas>(i).a = static_cast<float>(i * 0.1);

			nb_of_components += 2;

			if (counter_3 == 3)
			{
				pool.add<health>(i).points = i;
				nb_of_components++;
				counter_3 = -1;
			}

			if (counter_5 == 5)
			{
				pool.add<attack>(i).damage = i * 10;
				nb_of_components++;
				counter_5 = -1;
			}

			if (counter_7 == 7)
			{
				pool.add<defense>(i).armor = i * 5;
				nb_of_components++;
				counter_7 = -1;
			}

			counter_3++;
			counter_5++;
			counter_7++;
		}
		auto end = std::chrono::high_resolution_clock::now();
		std::cout << "took: "
			<< std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
			<< " us\n";

		std::cout << "Emplacing " << nb_of_components << " components to " << entity_count << " entities ";

		start = std::chrono::high_resolution_clock::now();
		pool.emplace_commands();
		end = std::chrono::high_resolution_clock::now();

		std::cout << "took: "
			<< std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
			<< " us\n";

		// Measure time for querying multiple components
		start = std::chrono::high_resolution_clock::now();
		// Measure time for querying multiple components
		start = std::chrono::high_resolution_clock::now();
		auto query_1 = pool.query<position, velocity, health>();
		for (auto q : query_1)
		{
			position& pos = q.get<position>();
			velocity& vel = q.get<velocity>();
			health& hp = q.get<health>();

			// Simulate some system work with the queried components
			pos.x += vel.vx;
		}
		auto query_2 = pool.query<position, velocity, attack>();
		for (auto q : query_2)
		{
			position& pos = q.get<position>();
			velocity& vel = q.get<velocity>();
			attack& atk = q.get<attack>();

			// Simulate some system work with the queried components
			pos.x += vel.vx * atk.damage;
		}
		auto query_3 = pool.query<position, velocity, defense>();
		for (auto q : query_3)
		{
			position& pos = q.get<position>();
			velocity& vel = q.get<velocity>();
			defense& def = q.get<defense>();

			// Simulate some system work with the queried components
			vel.vx -= def.armor * pos.x;
		}
		auto query_4 = pool.query<position, velocity, health, attack, defense>();
		end = std::chrono::high_resolution_clock::now();
		std::cout << "Querying multiple components took: "
			<< std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
			<< " us\n";

		// Measure time for querying a larger system with multiple components
		start = std::chrono::high_resolution_clock::now();
		for (auto q : query_4)
		{
			position& pos = q.get<position>();
			velocity& vel = q.get<velocity>();
			health& hp = q.get<health>();
			attack& atk = q.get<attack>();
			defense& def = q.get<defense>();

			// Simulate some system work with the queried components
			pos.x += vel.vx;
			hp.points -= atk.damage;
			def.armor += atk.damage / 2; // Simulate defense affecting armor
		}
		end = std::chrono::high_resolution_clock::now();
		std::cout << "Querying larger system took: "
			<< std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
			<< " us\n";

		// Measure time for removing components
		start = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < entity_count; i ++)
		{
			pool.remove_component<health>(i);
			pool.remove_component<attack>(i);
			pool.remove_component<position>(i);
			pool.remove_component<velocity>(i);
			pool.remove_component<defense>(i);
		}
		pool.emplace_commands();

		end = std::chrono::high_resolution_clock::now();
		std::cout << "Removing components took: "
			<< std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
			<< " us\n";
	}

	for (int i = 0; i < entity_count; i++)
	{
		while (pool.has<pingas>(i))
		{
			pool.remove_component<pingas>(i);
			pool.emplace_commands();
		}
	}
	std::cout << "Done" << std::endl;
	while (true) {  }
}

int main()
{
	peetcs::archetype_pool pool;

	pipo::init();
	pipo::create_window(1200, 800, "Hello World");

	pipo::shader::allocate_settings shader_settings = {};
	char vertex_code[] = R"(	
#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
})";

	char fragment_code[] = R"(	
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
} )";


	shader_settings.vertex_shader_code = vertex_code;
	shader_settings.fragment_shader_code = fragment_code;

	pipo::mesh::allocate_settings mesh_settings = {};

	float vertices[] = {
	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
	 0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
	 0.0f,  0.5f, 0.0f, 0.0f, 0.0f,
	};

	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 2,
	};

	mesh_settings.layout = pipo::mesh::vertex_3d;
	mesh_settings.vertices = (unsigned char*)vertices;
	mesh_settings.indices = (unsigned char*)indices;
	mesh_settings.nb_of_indices = 3;
	mesh_settings.nb_of_vertices = 3;

	peetcs::entity_id triangle = 1;
	peetcs::entity_id camera = 0;

	pipo::shader_id shader = pipo::resources::create_shader_gpu(shader_settings);
	{
		int amount = 40000;
		for (int i = 0; i < amount; i++)
		{
			triangle = i + 1;

			int x = i % (int)sqrt(amount) - sqrt(amount) / 2;
			int y = i / (int)sqrt(amount) - sqrt(amount) / 2;

			pipo::mesh_render_data& mesh_render = pool.add<pipo::mesh_render_data>(triangle);
			mesh_render.mesh_id = pipo::resources::allocate_mesh_gpu(mesh_settings);
			mesh_render.visible = i % 10 == 0;

			pipo::material_data& material = pool.add<pipo::material_data>(triangle);
			material.program = shader;

			pipo::transform_data& triangle_transform = pool.add<pipo::transform_data>(triangle);
			triangle_transform.set_pos(x, y, 0 );
			triangle_transform.set_rotation(x,y, 0);
			triangle_transform.set_scale(1, 1, 1);

			pool.emplace_commands();
		}



		pipo::camera_data& camera_data = pool.add<pipo::camera_data>(camera);
		pipo::transform_data& camera_transform = pool.add<pipo::transform_data>(camera);

		camera_transform.set_pos(0, 0, 2);
		camera_transform.set_rotation(0, 0, 0);

		camera_data.c_near = 0.1f;
		camera_data.c_far = 1000.0f;
		camera_data.aspect = 800.0f / 600.0f; // For example
		camera_data.fov = 60.0f;
		camera_data.type = pipo::view_type::perspective;
		camera_data.active = true;

		pool.emplace_commands();
	}

	while (pipo::start_frame())
	{
		pipo::render_frame(pool);

		auto camera_query = pool.query<pipo::camera_data, pipo::transform_data>();
		for (auto camera_value : camera_query)
		{
			pipo::transform_data& camera_transform = camera_value.get<pipo::transform_data>();
			camera_transform.position[0] += 0.001f;
			camera_transform.position[1] += 0.001f;
			camera_transform.position[2] += 0.100f;
			camera_transform.rotation[2] -= 0.001f;
		}

		auto mesh_query = pool.query<pipo::mesh_render_data, pipo::transform_data>();
		for (auto query : mesh_query)
		{
			pipo::transform_data& transform = query.get<pipo::transform_data>();
			transform.rotation[2] += 1.f;
		}
	}

	pipo::deinit();

	return 0;
}
