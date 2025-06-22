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


	struct rigid_body_data
	{
		static constexpr int id = lib_id + 1;
	};


	struct sphere_collider_data
	{

	};

	struct box_collider_data
	{
		static constexpr int id = lib_id +2;

		pipo::transform_data transform;

		float radius_from_center = 1.f;
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

	static aabb get_collider_aabb(peetcs::archetype_pool& pool, const box_collider_data& collider_a,
	                              const pipo::transform_data& transform_a);
	static void tick(peetcs::archetype_pool& pool, pipo& gpu_context);


	struct sat
	{
		/*template<typename VectorType>
		struct projected_minmax
		{
			float min;
			float max;

			VectorType min_position;
			VectorType max_position;
		};

		template<typename VectorType>
		struct sphere
		{
			float radius;
			VectorType position;
		};

		struct raycast_mesh
		{
			raycast_mesh(glm::vec3 start, glm::vec3 end)
			{
				vertices = { glm::vec3(start), glm::vec3(end) };

				glm::vec3 forward = glm::normalize((end - start));
				glm::vec3 up = glm::vec3(0, 1, 0);
				glm::vec3 right = glm::normalize(glm::cross(forward, up));
				up = glm::normalize(glm::cross(right, forward));

				axes = { up, right, forward };
			}

			std::array<glm::vec3, 2> vertices;
			std::array<glm::vec3, 3> axes;
		};*/

		/*template<typename VectorType>
		projected_minmax<VectorType> project_on_axis(const sphere<VectorType>& sphere, const VectorType axis)
		{
			VectorType position_a = sphere.position + axis * sphere.radius;
			VectorType position_b = sphere.position + axis * sphere.radius * -1.f;
			auto a = sphere.position.dot(axis) + sphere.radius;
			auto b = sphere.position.dot(axis) - sphere.radius;

			if (a > b)
			{
				return projected_minmax<VectorType>{ b, a, position_b, position_a };
			}
			else
			{
				return projected_minmax<VectorType>{ a, b, position_a, position_b };
			}
		}*/


		/*
		static projected_minmax<glm::vec3> project_on_axis(const std::vector<glm::vec3>& shape, const glm::vec3 axis);


		static auto get_combined_axes(const mesh_container& a, const mesh_container& b);

		static collision_data test_collision(const mesh_container& a, const mesh_container& b) const;*/

		/*template<class MeshContainerB, typename VectorType>
		collision_data test_collision(const sphere<VectorType>& a, const MeshContainerB& b)
		{
			const auto& axes_to_test = b.axes;
			VectorType resolution_vector = { 0, 0, 0 };
			VectorType resolve_axis = { 0, 0, 0 };
			float min_overlap = std::numeric_limits<float>::max();
			vector3<float> collision_position;
			for (auto axis : axes_to_test)
			{
				axis = axis.normalized();

				if (axis == VectorType{ 0, 0, 0 })
				{
					continue;
				}

				projected_minmax<VectorType> projection_a = project_on_axis(a, axis);
				projected_minmax<VectorType> projection_b = project_on_axis(b.vertices, axis);

				if (projection_a.min > projection_b.max || projection_b.min > projection_a.max)
				{
					return {};
				}

				VectorType position;
				float overlap_a = projection_b.max - projection_a.min;
				float overlap_b = projection_b.min - projection_a.max;
				float overlap = std::abs(overlap_a) > std::abs(overlap_b) ? overlap_b : overlap_a;

				if (std::abs(projection_b.max - projection_a.min) < std::abs(projection_b.min - projection_a.max))
				{
					position = projection_a.min_position;
				}
				else
				{
					position = projection_a.max_position;
				}

				if (std::abs(overlap) <= std::abs(min_overlap))
				{
					min_overlap = overlap;
					resolve_axis = axis;
					collision_position = position;
				}
			}

			resolution_vector = resolve_axis * min_overlap;

			// Apply the resolve to the collision
			return
			{
				.collision = true,
				.resolve_offset = resolution_vector,
				.collision_position = collision_position,  // Add the collision position to the result
				.normal = resolution_vector.normalized()
			};
		}

		template<typename VectorType>
		collision_data test_collision(const sphere<VectorType>& a, const sphere<VectorType>& b)
		{
			const auto& axes_to_test = { a.position - b.position };
			VectorType resolution_vector = { 0, 0, 0 };
			VectorType resolve_axis = { 0, 0, 0 };
			float min_overlap = std::numeric_limits<float>::max();
			vector3<float> collision_position;
			for (auto axis : axes_to_test)
			{
				axis = axis.normalized();

				if (axis == VectorType{ 0, 0, 0 })
				{
					continue;
				}

				projected_minmax<VectorType> projection_a = project_on_axis(a, axis);
				projected_minmax<VectorType> projection_b = project_on_axis(b, axis);

				if (projection_a.min > projection_b.max || projection_b.min > projection_a.max)
				{
					return {};
				}

				VectorType position;
				float overlap_a = projection_b.max - projection_a.min;
				float overlap_b = projection_b.min - projection_a.max;
				float overlap = std::abs(overlap_a) > std::abs(overlap_b) ? overlap_b : overlap_a;

				if (std::abs(projection_b.max - projection_a.min) < std::abs(projection_b.min - projection_a.max))
				{
					position = projection_a.min_position;
				}
				else
				{
					position = projection_a.max_position;
				}

				if (std::abs(overlap) <= std::abs(min_overlap))
				{
					min_overlap = overlap;
					resolve_axis = axis;
					collision_position = position;
				}
			}

			resolution_vector = resolve_axis * min_overlap;

			// Apply the resolve to the collision
			return
			{
				.collision = true,
				.resolve_offset = resolution_vector,
				.collision_position = collision_position,  // Add the collision position to the result
				.normal = resolution_vector.normalized()
			};
		}*/
	};
};

/*
inline phesycs_impl::sat::projected_minmax<glm::vec<3, float>> phesycs_impl::sat::project_on_axis(
	const std::vector<glm::vec3>& shape, const glm::vec3 axis)
{
	float min_proj = INFINITY;
	float max_proj = -INFINITY;

	glm::vec3 min_position;
	glm::vec3 max_position;

	for (const auto& vertex : shape)
	{
		const float proj = glm::dot(vertex, axis);

		if (proj < min_proj)
		{
			min_proj = proj;
			min_position = vertex;
		}

		if (proj > max_proj)
		{
			max_proj = proj;
			max_position = vertex;
		}
	}

	return projected_minmax{.min = min_proj, .max = max_proj, .min_position = min_position, .max_position = max_position };
}

inline auto phesycs_impl::sat::get_combined_axes(const mesh_container& a, const mesh_container& b)
{
	constexpr size_t a_size = a.axes.size();
	constexpr size_t b_size = b.axes.size();
	constexpr size_t axis_size = a_size + b_size + a_size * b_size;

	std::vector<glm::vec3> axes = {};
	axes.reserve(axis_size);

	size_t index = 0;
	for (auto& axis : a.axes)
	{
		axes.push_back(glm::normalize(axis));
	}

	for (auto& axis : b.axes)
	{
		axes.push_back(glm::normalize(axis));
	}

	for (auto& axis_a : a.axes)
	{
		for (auto& axis_b : b.axes)
		{
			axes.push_back(glm::cross(axis_a, axis_b));
		}
	}

	return axes;
}

inline phesycs_impl::collision_data phesycs_impl::sat::test_collision(const mesh_container& a, const mesh_container& b) const
{
	const auto axes_to_test = get_combined_axes(a, b);
	glm::vec3 average_resolve = { 0, 0, 0 };
	glm::vec3 resolution_vector = { 0, 0, 0 };
	float min_overlap = std::numeric_limits<float>::max();
	int tested_axes = 0;
	glm::vec3 collision_position;
	for (auto axis : axes_to_test)
	{
		if (axis == glm::vec3{ 0, 0, 0 })
		{
			continue;
		}

		projected_minmax<glm::vec3> projection_a = project_on_axis(a.vertices, axis);
		projected_minmax<glm::vec3> projection_b = project_on_axis(b.vertices, axis);

		if (projection_a.min > projection_b.max || projection_b.min > projection_a.max)
		{
			return {};
		}

		float overlap_a = projection_b.max - projection_a.min;
		float overlap_b = projection_a.max - projection_b.min;


		float overlap = (std::abs(overlap_a) < std::abs(overlap_b)) ? overlap_a : -overlap_b;

		if (std::abs(overlap) < std::abs(min_overlap))
		{
			min_overlap = overlap;
			resolution_vector = axis * overlap;

			// Fully contained collision a in b
			if (projection_b.min < projection_a.min && projection_a.max < projection_b.max)
			{
				collision_position = (projection_a.min_position + projection_a.max_position) / 2;
			}
			// Fully contained collision b in a
			else if (projection_a.min < projection_b.min && projection_b.max < projection_a.max)
			{
				collision_position = (projection_b.min_position + projection_b.max_position) / 2;
			}
			// Edge contained collision a in b min
			else if (projection_b.min < projection_a.min && projection_b.max < projection_a.max)
			{
				collision_position = projection_b.max_position;
			}
			// Edge contained collision b in a min
			else if (projection_a.min < projection_b.min && projection_a.max < projection_b.max)
			{
				collision_position = projection_a.max_position;
			}

			//collision_position = (collision_position * 2 + resolution_vector) / 2;
		}

		average_resolve += axis * overlap;
		tested_axes++;
	}*/


	// Calculate the collision position (point of contact) for object a
	// For simplicity, take the center of object a and move it along the resolve axis
	// by half of the minimum resolve.
	/*vector3<float> collision_position = average_resolve / tested_axes;*/  // Halfway into the overlap

	// Apply the resolve to the collision
	/*
	return
	{
		.collision = true,
		.resolve_offset = resolution_vector,
		.collision_position = collision_position,  // Add the collision position to the result
		.normal = glm::normalize(resolution_vector)
	};
}
*/

