#include <iostream>
#include <chrono>
#include "include/archetype_pool.hpp"

struct position
{
	float x, y, z;
};

struct velocity
{
	float vx, vy, vz;
};

struct health
{
	int points;
};

struct attack
{
	int damage;
};

struct defense
{
	int armor;
};

int main()
{
	constexpr int entity_count = 100000; // Number of entities for the test
	peetcs::archetype_pool pool;

	// Measure time for adding components
	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < entity_count; ++i)
	{
		pool.add<position>(i).x = static_cast<float>(i);
		pool.add<velocity>(i).vx = static_cast<float>(i * 0.1);

		if (i % 3 == 0)
		{
			pool.add<health>(i).points = i;
		}

		if (i % 5 == 0)
		{
			pool.add<attack>(i).damage = i * 10;
		}

		if (i % 7 == 0)
		{
			pool.add<defense>(i).armor = i * 5;
		}
	}
	pool.emplace_commands();
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "Adding components took: "
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
	for (int i = 0; i < entity_count; i += 4)
	{
		pool.remove<health>(i);
		pool.remove<attack>(i);
	}
	pool.emplace_commands();
	end = std::chrono::high_resolution_clock::now();
	std::cout << "Removing components took: "
		<< std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
		<< " us\n";

	return 0;
}
