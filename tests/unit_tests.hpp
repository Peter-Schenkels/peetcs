#pragma once
#include <cmath>
#include <iostream>
#include "include/archetype_pool.hpp"

struct position
{
	constexpr static uint16_t id = 10001;
	float x, y, z;
};

struct velocity
{
	constexpr static uint16_t id = 10002;
	float vx, vy, vz;
};

struct health
{
	constexpr static uint16_t id = 10003;
	int points;
};

struct attack
{
	constexpr static uint16_t id = 10005;
	int damage;
};

struct defense
{
	constexpr static uint16_t id = 10004;
	int armor;
};


// template <typename T, typename... Args>
// class can_to_string
// {
// 	template <typename C, typename = decltype(std::to_string(std::declval<Args>()...))>
// 	static std::true_type test(int);
// 	template <typename C>
// 	static std::false_type test(...);

// public:
// 	static constexpr bool value = decltype(test<T>(0))::value;
// };

template<typename A, typename B>
void assert_eq(A a, B b)
{
	bool failed = false;

	if constexpr ((std::is_same_v<A, float> || std::is_same_v<A, double>) &&
                  (std::is_same_v<B, float> || std::is_same_v<B, double>))
	{
		if (std::numeric_limits<float>::epsilon() * 10 < std::abs(a - b))
		{
			failed = true;
		}
	}
	else if (a != b)
	{
		failed = true;
	}

	if (failed)
	{
		std::string a_name = "A";
		// if constexpr (can_to_string<A>::value)
		// {
		// 	a_name = std::to_string(a);
		// }

		std::string b_name = "B";
		// if constexpr  (can_to_string<B>::value)
		// {
		// 	b_name = std::to_string(b);
		// }

		std::cout << a_name << " != " << b_name << std::endl;
		halt();
	}
}

template<typename A, typename B>
void assert_neq(A a, B b)
{
	bool failed = false;

	if constexpr ((std::is_same_v<A, float> || std::is_same_v<A, double>) &&
                  (std::is_same_v<B, float> || std::is_same_v<B, double>))
	{
		if (std::numeric_limits<A>::epsilon() >= std::abs(a - b))
		{
			failed = true;
		}
	}
	else if (a == b)
	{
		failed = true;
	}

	if (failed)
	{
		std::string a_name = "A";
		// if constexpr (can_to_string<A>::value)
		// {
		// 	a_name = std::to_string(a);
		// }

		std::string b_name = "B";
		// if constexpr  (can_to_string<B>::value)
		// {
		// 	b_name = std::to_string(b);
		// }

		std::cout << a_name << " == " << b_name << std::endl;
		halt();
	}
}

/*
inline void test_storage_constructor_default()
{
	generic_container s;
	assert_eq(s.size(), 0);
	std::cout << "test_constructor_default passed\n";
}

inline void test_generic_container_constructor_with_type_description()
{
	std::vector<element_layout::type_info>  types = { element_layout::type_info::init<int>() };
	generic_container s(types, 5);
	assert_eq(s.size(), 0);
	std::cout << "test_constructor_with_type_description passed\n";
}

inline void test_generic_container_add_element()
{
	std::vector<element_layout::type_info> types = {
		element_layout::type_info::init<int>(),
		element_layout::type_info::init<float>(),
		element_layout::type_info::init<char>()
	};
	generic_container s(types, 5);

	{
		auto region = s.add_element(1);
		assert_eq(region.get_id(), 1);
		assert_eq(s.size(), 1);

		int value_int = 124;
		region.set(value_int);

		float value_float = 12.333f;
		region.set(value_float);

		char value_char = 'f';
		region.set(value_char);


		assert_eq(region.get<int>(), value_int);
		assert_eq(region.get<float>(), value_float);
		assert_eq(region.get<char>(), value_char);

		auto region_2 = s.add_element(2);
		assert_eq(region_2.get_id(), 2);
		assert_eq(s.size(), 2);

		int value_int_2 = 123;
		region_2.set(value_int_2);

		float value_float_2 = 11.333f;
		region_2.set(value_float_2);

		char value_char_2 = 'a';
		region_2.set(value_char_2);

		assert_eq(region_2.get<int>(), value_int_2);
		assert_eq(region_2.get<float>(), value_float_2);
		assert_eq(region_2.get<char>(), value_char_2);

		auto region_get_2 = s.get_element(2);
		assert_eq(region_get_2.get<int>(), value_int_2);
		assert_eq(region_get_2.get<float>(), value_float_2);
		assert_eq(region_get_2.get<char>(), value_char_2);

		auto region_get_1 = s.get_element(1);

		assert_eq(region_get_1.get<int>(), value_int);
		assert_eq(region_get_1.get<float>(), value_float);
		assert_eq(region_get_1.get<char>(), value_char);


		region_get_1.copy_sub_elements(region_get_2);

		assert_eq(region_get_1.get<int>(), value_int_2);
		assert_eq(region_get_1.get<float>(), value_float_2);
		assert_eq(region_get_1.get<char>(), value_char_2);

	}

	std::cout << "test_add_element passed\n";
}

inline void test_generic_container_get_element()
{
	std::vector<element_layout::type_info>  types = { element_layout::type_info::init<int>() };
	generic_container s(types, 5);

	{
		s.add_element(1);
		auto region_1 = s.get_element_at(0);
		assert_eq(region_1.get_id(), 1);

		auto region_2 = s.get_element(1);
		assert_eq(region_2.get_id(), 1);
	}

	{
		s.add_element(2);
		auto region_1 = s.get_element_at(1);
		assert_eq(region_1.get_id(), 2);
		assert_eq(region_1.get_index(), 1);

		auto region_2 = s.get_element(2);
		assert_eq(region_2.get_id(), 2);
		assert_eq(region_2.get_index(), 1);
	}

	std::cout << "test_get_element passed\n";
}

inline void test_generic_container_remove_element()
{
	std::vector<element_layout::type_info>  types = { element_layout::type_info::init<int>() };
	generic_container s(types, 5);


	int nb_of_elements = 10;

	for(int i = 0; i < nb_of_elements; i++)
	{
		s.add_element(i + 10);
	}

	int last_element_id = s.last().get_id();
	int id_element      = s.get_element(13).get_id();
	int index_element   = s.get_element(13).get_index();
	s.remove(id_element);

	assert_eq(s.size(), nb_of_elements - 1);
	assert_eq(s.contains(13), false);
	assert_neq(s.get_element(last_element_id).get_id(), s.last().get_id());

	std::cout << "test_remove_element passed\n";
}

inline void test_generic_container_size()
{
	std::vector<element_layout::type_info>  types = { element_layout::type_info::init<int>() };
	generic_container s = {types, 5};

	assert_eq(s.size(), 0);
	s.add_element(1);
	assert_eq(s.size(), 1);
	s.add_element(2);
	assert_eq(s.size(), 2);
	std::cout << "test_size passed\n";
}

inline void test_generic_container_last()
{
	std::vector<element_layout::type_info>  types = { element_layout::type_info::init<int>() };
	generic_container s(types, 5);

	s.add_element(1);
	s.add_element(2);
	assert_eq(s.last().get_id(), 2); // Last index is size() - 1
	assert_eq(s.last().get_index(), 1); // Last index is size() - 1
	std::cout << "test_last passed\n";
}

inline void test_generic_container_iterator_begin_end()
{
	std::vector<element_layout::type_info>  types = { element_layout::type_info::init<int>() };
	generic_container s(types, 5);

	s.add_element(1);
	s.add_element(2);

	generic_container::iterator it = s.begin();
	assert_eq((*it).get_id(), 1);
	++it;
	assert_neq(it, s.end());
	assert_eq((*it).get_id(), 2);
	++it;
	assert_eq(it, s.end());
	std::cout << "test_iterator_begin_end passed\n";
}

inline void test_generic_container_iterator_equality() 
{
	std::vector<element_layout::type_info>  types = { element_layout::type_info::init<int>() };
	generic_container s(types, 5);

	s.add_element(1);
	s.add_element(2);

	auto it1 = s.begin();
	auto it2 = s.begin();
	assert_eq(it1, it2);

	++it1;
	assert_neq(it1, it2);
	std::cout << "test_iterator_equality passed\n";
}
*/


inline void test_add_components()
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
		position* pos = pool.get_from_owner<position>(i);
		velocity* vel = pool.get_from_owner<velocity>(i);

		assert_eq(pos->x, static_cast<float>(i));
		assert_eq(vel->vx, static_cast<float>(i * 0.1));
	}
	std::cout << "test_get_components_before_emplace passed\n";


	pool.emplace_commands();

	for (int i = 0; i < 100; ++i) {
		position* pos = pool.get_from_owner<position>(i);
		velocity* vel = pool.get_from_owner<velocity>(i);

		assert_eq(pos->x, static_cast<float>(i));
		assert_eq(vel->vx, static_cast<float>(i * 0.1));
	}
	std::cout << "test_get_components_after_emplace passed\n";
}

inline void test_query_components()
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

	auto query_2 = pool.query<position, health>();
	for (auto q : query_2)
	{
		position& pos = q.get<position>();
		velocity& vel = q.get<velocity>();

		//assert_eq(pos.x, hp.points / 10.0f);
		assert_eq(vel.vx, pos.x * 0.1f);
	}

	auto query_3 = pool.query<position>();
	for (auto q : query_2)
	{
		position& pos = q.get<position>();

		//assert_eq(pos.x, hp.points / 10.0f);
		assert_eq(pos.x * 0.1f, pos.x * 0.1f);
	}


	for (int i = 0; i < 50; ++i) 
	{
		pool.remove_component<position>(i);
		pool.remove_component<velocity>(i);
		if (i % 2 == 0)
			pool.remove_component<health>(i);
	}

	pool.emplace_commands();

	std::cout << "test_query_components passed\n";
}

inline void test_remove_components()
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
		pool.remove_component<health>(i);
	}

	auto query = pool.query<position, health>();
	//assert_eq(query.empty()); // All health components should be removed

	std::cout << "test_remove_components passed\n";
}


inline void test_large_system()
{
	constexpr int entity_count = 100000;
	peetcs::archetype_pool pool;

	for (int i = 0; i < entity_count; ++i) {
		auto& position_data = pool.add<position>(i);

		position_data.x = static_cast<float>(i);
		auto& velocity_data = pool.add<velocity>(i);
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

inline void test_edge_cases()
{
	peetcs::archetype_pool pool;

	// Test adding the same component multiple times
	pool.add<position>(0).x = 1.0f;

	// Readding component what should be the behaviour?
	pool.add<position>(0).x = 2.0f;

	auto position_data = pool.get_from_owner<position>(0);

	assert_eq(position_data->x, 1.0f);


	// Test removing non-existent component
	try {
		pool.remove_component<velocity>(0);
		std::cout << "No exception on removing non-existent component\n";
	}
	catch (...) {
		//assert_eq(false && "Exception thrown on removing non-existent component");
	}

	// Test querying non-existent components
	auto query = pool.query<velocity>();
	//assert_eq(query.empty());

	std::cout << "test_edge_cases passed\n";
}

void test_component_lists()
{
	peetcs::archetype_pool pool;
	peetcs::entity_id entity = 0;

	float value_1 = 0.1f;
	float value_2 = 0.2f;
	pool.add<position>(entity).x = value_1;
	pool.emplace_commands();

	pool.add<position>(entity).x = value_2;
	pool.emplace_commands();

	assert_eq(pool.get_from_owner<position>(entity)->x,value_1);
	assert_eq(pool.get_at<position>(entity, 1)->x, value_2);
	assert_eq(pool.get_at<position>(entity, 0)->x, value_1);

	assert_eq(pool.has<position>(entity), true);
	pool.remove_component<position>(entity);
	pool.emplace_commands();

	assert_eq(pool.get_from_owner<position>(entity)->x, value_2);
	assert_eq(pool.has<position>(entity), true);
	pool.remove_component<position>(entity);
	pool.emplace_commands();

	assert_eq(pool.has<position>(entity), false);

	std::cout << "test_component_lists passed\n";
}

int inline run_tests()
{
	/*test_storage_constructor_default();
	test_generic_container_constructor_with_type_description();
	test_generic_container_add_element();
	test_generic_container_get_element();
	test_generic_container_remove_element();
	test_generic_container_size();
	test_generic_container_last();
	test_generic_container_iterator_begin_end();
	test_generic_container_iterator_equality();*/

	test_add_components();
	test_query_components();
	test_remove_components();
	test_edge_cases();
	test_component_lists();
	//test_large_system();

	return 0;
}
