#include <iostream>

#include "include/archetype_pool.hpp"
#include "include/query_value.hpp"

struct component_test
{
	float test = 1;
};

struct position
{
	float x = 2;
	float y = 31;
	float z = 1;
};

struct health
{
	int points = 100;
};

int main()
{
	peetcs::archetype_pool pool;


	// System tick
	{
		// Creates: Add <Position> command with temp data -> results in: archetype <Position> creation
		position& position_data_0   = pool.add<position>(0);
		position_data_0.x = 0;
		position_data_0.y = 0;
		position_data_0.z = 0;

		// Creates: Add <Health> command with temp data -> results in: archetype <Health, Position> creation
		health& health_data_0       = pool.add<health>(0);
		health_data_0.points = 1;

		// Creates: Add <ComponentTest> command with temp data -> results in: archetype <Health, Position, ComponentTest> creation
		component_test& test_data_0 = pool.add<component_test>(0);

		// Creates: Add <Position> command with temp data -> results in: archetype <Position> creation
		position& position_data_1   = pool.add<position>(1);
		position_data_1.x = 1;
		position_data_1.y = 2;
		position_data_1.z = 3;

		// Creates: Add <ComponentTest> command with temp data -> results in: archetype <Position, ComponentTest> creation
		component_test& test_data_1 = pool.add<component_test>(1);
		test_data_1.test = 43;


		// Creates: Add <Position> command with temp data -> results in: archetype <Position> creation
		position& position_data_2   = pool.add<position>(2);

		// Creates: Add <Health> command with temp data -> results in: archetype <Health, Position> creation
		health& health_data = pool.add<health>(2);

		// Gets data from "Add entity to <position>" command (Always Expensive Get)
		position& position_data = *pool.get<position>(2);
	}

	// System tick done (Apply all commands)
	pool.emplace_commands();

	// All component archetypes are now created contiguously

	// New system tick
	{
		// Add command: Remove <health> from entity 2 ->  results in: archetype <Position> migration
		pool.remove<health>(2);

		// Get position from <position, health> table (Still expensive but cheaper depending on context)
		pool.emplace_commands();

		auto query = pool.query<position, health>();

		for (peetcs::query_value query_value : query)
		{
			position& position_data = query_value.get<position>();
			std::cout << position_data.x << " " << position_data.y << " " << position_data.z << " " << std::endl;
		}
	}
}