#include <cassert>
#include <iostream>
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



#define assert_eq(a, b) \
	if ((a) != (b)) { std::cout << std::to_string((a)) << " != " << std::to_string((b)) << std::endl; __debugbreak(); } #a#b\

#define assert_neq(a, b) \
	if ((a) == (b)) { std::cout << std::to_string((a)) << " == " << std::to_string((b)) << std::endl; __debugbreak(); } #a#b\

#define assert_eq_no_print(a, b) \
	if ((a) != (b)) { std::cout << "a" << " != " << "b" << std::endl; __debugbreak(); } #a#b\

#define assert_neq_no_print(a, b) \
	if ((a) == (b)) { std::cout << "a" << " == " << "b" << std::endl; __debugbreak(); } #a#b\


using namespace peetcs;

void test_storage_constructor_default()
{
	storage s;
	assert_eq(s.size(), 0);
	std::cout << "test_constructor_default passed\n";
}

void test_storage_constructor_with_type_description()
{
	std::vector types = { peetcs::type_info{std::type_index(typeid(int)), sizeof(int)} };
	storage s(types, 5);
	assert_eq(s.size(), 0);
	std::cout << "test_constructor_with_type_description passed\n";
}

void test_storage_add_element()
{
	std::vector types = { peetcs::type_info{std::type_index(typeid(int)), sizeof(int)} };
	storage s(types, 5);

	auto region = s.add_element(1);
	assert_eq(region.identifier, 1);
	assert_eq(s.size(), 1);
	std::cout << "test_add_element passed\n";
}

void test_storage_get_element()
{
	std::vector types = { peetcs::type_info{std::type_index(typeid(int)), sizeof(int)} };
	storage s(types, 5);

	{
		s.add_element(1);
		auto region = s.get_element(0);
		assert_eq(region.identifier, 1);
	}

	{
		s.add_element(2);
		auto region = s.get_element(1);
		assert_eq(region.identifier, 2);
	}

	std::cout << "test_get_element passed\n";
}

void test_storage_remove_element()
{
	std::vector types = { peetcs::type_info{std::type_index(typeid(int)), sizeof(int)} };
	storage s(types, 5);

	s.add_element(1);
	s.remove_element(0);
	assert_eq(s.size(), 0);
	std::cout << "test_remove_element passed\n";
}

void test_storage_size()
{
	std::vector types = { peetcs::type_info{std::type_index(typeid(int)), sizeof(int)} };
	storage s(types, 5);

	assert_eq(s.size(), 0);
	s.add_element(1);
	assert_eq(s.size(), 1);
	s.add_element(2);
	assert_eq(s.size(), 2);
	std::cout << "test_size passed\n";
}

void test_storage_last()
{
	std::vector types = { peetcs::type_info{std::type_index(typeid(int)), sizeof(int)} };
	storage s(types, 5);

	s.add_element(1);
	s.add_element(2);
	assert_eq(s.last(), 1); // Last index is size() - 1
	std::cout << "test_last passed\n";
}

void test_storage_iterator_begin_end()
{
	std::vector types = { peetcs::type_info{std::type_index(typeid(int)), sizeof(int)} };
	storage s(types, 5);

	s.add_element(1);
	s.add_element(2);

	auto it = s.begin();
	assert_eq((*it).identifier, 1);
	++it;
	assert_neq_no_print(it, s.end());
	assert_eq((*it).identifier, 2);
	++it;
	assert_eq_no_print(it, s.end());
	std::cout << "test_iterator_begin_end passed\n";
}

void test_storage_iterator_equality() 
{
	std::vector types = { peetcs::type_info{std::type_index(typeid(int)), sizeof(int)} };
	storage s(types, 5);

	s.add_element(1);
	s.add_element(2);

	auto it1 = s.begin();
	auto it2 = s.begin();
	assert_eq_no_print(it1, it2);

	++it1;
	assert_neq_no_print(it1, it2);
	std::cout << "test_iterator_equality passed\n";
}


void test_add_components()
{
	peetcs::archetype_pool pool;

	for (int i = 0; i < 100; ++i) {
		position& pos = pool.add<position>(i);
		velocity& vel = pool.add<velocity>(i);

		pos.x = static_cast<float>(i);
		vel.vx = static_cast<float>(i * 0.1);

		assert_eq(pos.x, static_cast<float>(i));
		assert_eq(vel.vx, static_cast<float>(i * 0.1));
	}
	std::cout << "test_add_components passed\n";

	for (int i = 0; i < 100; ++i) {
		position* pos = pool.get<position>(i);
		velocity* vel = pool.get<velocity>(i);

		assert_eq(pos->x, static_cast<float>(i));
		assert_eq(vel->vx, static_cast<float>(i * 0.1));
	}
	std::cout << "test_get_components_before_emplace passed\n";


	pool.emplace_commands();

	for (int i = 0; i < 100; ++i) {
		position* pos = pool.get<position>(i);
		velocity* vel = pool.get<velocity>(i);

		assert_eq(pos->x, static_cast<float>(i));
		assert_eq(vel->vx, static_cast<float>(i * 0.1));
	}
	std::cout << "test_get_components_after_emplace passed\n";
}

void test_query_components()
{
	peetcs::archetype_pool pool;

	for (int i = 0; i < 50; ++i) {
		auto& position_data = pool.add<position>(i);
		position_data.x = static_cast<float>(i);

		auto& velocity_data = pool.add<velocity>(i);
		velocity_data.vx = static_cast<float>(i * 0.1);

		if (i % 2 == 0) pool.add<health>(i).points = i * 10;
	}

	pool.emplace_commands();

	auto query = pool.query<position, velocity, health>();
	for (auto q : query) 
	{
		position& pos = q.get<position>();
		velocity& vel = q.get<velocity>();
		health& hp = q.get<health>();

		assert_eq(pos.x, hp.points / 10.0f);
		assert_eq(vel.vx, pos.x * 0.1f);
	}

	std::cout << "test_query_components passed\n";
}

void test_remove_components()
{
	peetcs::archetype_pool pool;

	for (int i = 0; i < 50; ++i) {
		auto& pos = pool.add<position>(i);
		pos.x = static_cast<float>(i);
		if (i % 2 == 0)
		{
			auto& he = pool.add<health>(i);
			he.points = i * 10;
		}
	}



	pool.emplace_commands();

	for (int i = 0; i < 50; i += 2) {
		pool.remove<health>(i);
	}

	auto query = pool.query<position, health>();
	//assert_eq(query.empty()); // All health components should be removed

	std::cout << "test_remove_components passed\n";
}

void test_large_system()
{
	constexpr int entity_count = 100000;
	peetcs::archetype_pool pool;

	for (int i = 0; i < entity_count; ++i) {
		auto position_data = pool.add<position>(i);

		position_data.x = static_cast<float>(i);
		auto velocity_data = pool.add<velocity>(i);
		velocity_data.vx = static_cast<float>(i * 0.1);

		if (i % 3 == 0) pool.add<health>(i).points = i;
		if (i % 5 == 0) pool.add<attack>(i).damage = i * 10;
		if (i % 7 == 0) pool.add<defense>(i).armor = i * 5;
	}


	pool.emplace_commands();

	auto query = pool.query<position, velocity, health, attack, defense>();
	for (auto q : query) {
		position& pos = q.get<position>();
		velocity& vel = q.get<velocity>();
		health& hp = q.get<health>();
		attack& atk = q.get<attack>();
		defense& def = q.get<defense>();

		pos.x += vel.vx;
		hp.points -= atk.damage;
		def.armor += atk.damage / 2;
	}

	std::cout << "test_large_system passed\n";
}

void test_edge_cases()
{
	peetcs::archetype_pool pool;

	// Test adding the same component multiple times
	pool.add<position>(0).x = 1.0f;
	pool.add<position>(0).x = 2.0f;

	auto position_data = pool.get<position>(0);

	assert_eq(position_data->x, 2.0f);


	// Test removing non-existent component
	try {
		pool.remove<velocity>(0);
		std::cout << "No exception on removing non-existent component\n";
	}
	catch (...) {
		assert(false && "Exception thrown on removing non-existent component");
	}

	// Test querying non-existent components
	auto query = pool.query<velocity>();
	//assert_eq(query.empty());

	std::cout << "test_edge_cases passed\n";
}

int inline run_tests()
{

	test_storage_constructor_default();
	test_storage_constructor_with_type_description();
	test_storage_add_element();
	test_storage_get_element();
	test_storage_remove_element();
	test_storage_size();
	test_storage_last();
	test_storage_iterator_begin_end();
	test_storage_iterator_equality();

	test_add_components();
	test_query_components();
	test_remove_components();
	test_large_system();
	test_edge_cases();

	return 0;
}
