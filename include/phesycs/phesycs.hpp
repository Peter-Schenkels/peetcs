#pragma once
#include <array>
#include <cmath>
#include <vector>
#include <glm/vec3.hpp>

#include "include/archetype_pool.hpp"
#include "include/pipo/rasterizer.hpp"

struct aabb
{
	glm::vec3 min;
	glm::vec3 max;

	bool overlap(const glm::vec3& point) const;
	bool overlap(const aabb& other) const;
	bool inside(const aabb& other) const;
};


struct phesycs_impl
{
	static constexpr int lib_id = 1000;


	struct stor_vec3
	{
		float x, y, z;

		glm::vec3&& to_vec3()
		{
			return { x,y,z };
		}
	};

	static glm::vec3 to_vec3(const float arr[3]) {
		return glm::vec3(arr[0], arr[1], arr[2]);
	}


	struct sphere_collider_data
	{

	};

	struct box_collider_data
	{
		static constexpr int id = lib_id + 2;

		pipo::transform_data transform;

		std::array<glm::vec3, 8> vertices;
		aabb aabb;
	};

	struct rigid_body_data
	{
		static constexpr int id = lib_id + 1;

		pipo::transform_data transform;
		bool is_static = false;
		float velocity[3];
		float angular_velocity[3];
		float mass = 2;
		float inverse_mass = 1.f / 2.f;
		float inverse_inertia[3 * 3];


		enum class type
		{
			square
		};

		void inline set_mass(float mass, const box_collider_data& box_collider)
		{
			float w = box_collider.transform.scale[0];
			float h = box_collider.transform.scale[1];
			float d = box_collider.transform.scale[2];

			float x2 = h * h + d * d;
			float y2 = w * w + d * d;
			float z2 = w * w + h * h;

			float factor = (1.0f / 12.0f) * mass;

			this->inverse_mass = 1.f / mass;
			this->mass = mass;
			glm::mat3 inertia_tensor(
				factor * x2, 0, 0,
				0, factor * y2, 0,
				0, 0, factor * z2
			);

			glm::mat3 inverse_tensor = glm::inverse(inertia_tensor);
			inverse_inertia[0] = inverse_tensor[0][0];
			inverse_inertia[1] = inverse_tensor[1][0];
			inverse_inertia[2] = inverse_tensor[2][0];
			inverse_inertia[3] = inverse_tensor[0][1];
			inverse_inertia[4] = inverse_tensor[1][1];
			inverse_inertia[5] = inverse_tensor[2][1];
			inverse_inertia[6] = inverse_tensor[0][2];
			inverse_inertia[7] = inverse_tensor[1][2];
			inverse_inertia[8] = inverse_tensor[2][2];
		}

		void inline set_angular_velocity(const glm::vec3& in_velocity)
		{
			angular_velocity[0] = in_velocity[0];
			angular_velocity[1] = in_velocity[1];
			angular_velocity[2] = in_velocity[2];
		}

		void inline set_translational_velocity(const glm::vec3& in_velocity)
		{
			velocity[0] = in_velocity[0];
			velocity[1] = in_velocity[1];
			velocity[2] = in_velocity[2];
		}

		glm::vec3 inline get_velocity() const
		{
			return to_vec3(velocity);
		}

		glm::vec3 inline get_angular_velocity() const
		{
			return to_vec3(angular_velocity);
		}

		glm::mat3 inline get_inverse_inertia() const
		{
			const glm::mat3 mat(
				inverse_inertia[0], inverse_inertia[3], inverse_inertia[6],  // column 0
				inverse_inertia[1], inverse_inertia[4], inverse_inertia[7],  // column 1
				inverse_inertia[2], inverse_inertia[5], inverse_inertia[8]   // column 2
			);

			return mat;
		}
	};



	struct rectangle_collider_data
	{

	};

	struct circle_collider_data
	{

	};

	struct mesh_container
	{
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> axes;
	};

	struct collision_data
	{
		bool collision = false;
		glm::vec3 resolve_offset = {};
		glm::vec3 collision_position = {};
		glm::vec3 normal = {};
		glm::vec3 axis_a_normal = {};
		glm::vec3 axis_b_normal = {};
	};

	static void tick_collision_response(peetcs::archetype_pool& pool, pipo& gpu_context);
	static void tick_integration(peetcs::archetype_pool& pool, pipo& gpu_context);
};
