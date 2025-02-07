#include <iostream>
#include <chrono>
#include "include/archetype_pool.hpp"
#include "tests/unit_tests.hpp"


void run_performance_simulation_test(const int entity_count)
{
	peetcs::archetype_pool pool;
	for (int i = 0; i < 30; i++)
	{
		std::cout << "Adding components to " << entity_count << " entities";
		int nb_of_components = 0;
		// Measure time for adding components
		auto start = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < entity_count; ++i)
		{
			pool.add<position>(i).x = static_cast<float>(i);
			pool.add<velocity>(i).vx = static_cast<float>(i * 0.1);

			nb_of_components += 2;

			//if (i % 3 == 0)
			{
				pool.add<health>(i).points = i;
				nb_of_components++;
			}

			//if (i % 5 == 0)
			{
				pool.add<attack>(i).damage = i * 10;
				nb_of_components++;
			}

			//if (i % 7 == 0)
			{
				pool.add<defense>(i).armor = i * 5;
				nb_of_components++;
			}
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
		auto query_1 = pool.query<position, velocity, health>();
		auto query_2 = pool.query<position, velocity, attack>();
		auto query_3 = pool.query<position, velocity, defense>();
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
			pool.remove<health>(i);
			pool.remove<attack>(i);
			pool.remove<position>(i);
			pool.remove<velocity>(i);
			pool.remove<defense>(i);
		}
		pool.emplace_commands();
		end = std::chrono::high_resolution_clock::now();
		std::cout << "Removing components took: "
			<< std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
			<< " us\n";
	}
}

int main()
{
	run_tests();
	constexpr int entity_count = 1000; // Number of entities for the test
	run_performance_simulation_test(entity_count);

	return 0;
}
