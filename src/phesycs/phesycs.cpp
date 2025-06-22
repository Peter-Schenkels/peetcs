#include "include/phesycs/phesycs.hpp"

bool aabb::overlap(const glm::vec3& point) const
{
	return (min.x <= point.x) && (point.x < max.x) &&
           (min.y <= point.y) && (point.y < max.y) &&
           (min.z <= point.z) && (point.z < max.z);
}

bool aabb::overlap(const aabb& other) const
{
	return min.x <= other.max.x &&
		max.x >= other.min.x &&
		min.y <= other.max.y &&
		max.y >= other.min.y &&
		min.z <= other.max.z &&
		max.z >= other.min.z;
}

bool aabb::inside(const aabb& other) const
{
	return overlap(other.min) && overlap(other.max);
}

aabb phesycs_impl::get_collider_aabb(peetcs::archetype_pool& pool, const phesycs_impl::box_collider_data& collider_a, const pipo::transform_data& transform_a)
{
	aabb a_aabb = {};
	pipo::transform_data collider_transform = collider_a.transform;

	glm::mat4 translation = glm::translate(glm::mat4(1.0f), transform_a.get_pos());
	glm::mat4 scale_mat = glm::scale(glm::mat4(1.0f), transform_a.get_scale());

	glm::mat4 ws_matrix = translation * scale_mat;


	auto collider_matrix = ws_matrix * collider_transform.get_local_space();
	a_aabb.min = glm::vec3(collider_matrix * glm::vec4(collider_transform.get_pos() + collider_a.transform.get_scale() * -1.f, 1.f) );
	a_aabb.max = glm::vec3(collider_matrix * glm::vec4(collider_transform.get_pos() + collider_a.transform.get_scale() * 1.f, 1.f));

	if (a_aabb.min.x > a_aabb.max.x) std::swap(a_aabb.min.x, a_aabb.max.x);
	if (a_aabb.min.y > a_aabb.max.y) std::swap(a_aabb.min.y, a_aabb.max.y);
	if (a_aabb.min.z > a_aabb.max.z) std::swap(a_aabb.min.z, a_aabb.max.z);

	return a_aabb;
}

void phesycs_impl::tick(peetcs::archetype_pool& pool, pipo& gpu_context)
{
	struct collision_pair
	{
		int a_id;
		int b_id;

		inline bool operator==(const collision_pair& other) const
		{
			return a_id == other.b_id && b_id == other.a_id || a_id == other.a_id && b_id == other.b_id;
		}
	};

	auto query = pool.query<pipo::transform_data, box_collider_data, rigid_body_data>();


	std::vector<aabb> aabbs = {};
	std::vector<collision_pair> pairs = {};

	int a_id = -1;
	for (auto body_a : query)
	{
		box_collider_data collider_a = body_a.get<box_collider_data>();
		pipo::transform_data transform_a = body_a.get<pipo::transform_data>();

		if (transform_a.parent == peetcs::invalid_entity_id)
		{
			a_id++;
			aabb a_aabb;
			if (aabbs.size() <= a_id)
			{
				a_aabb = get_collider_aabb(pool, collider_a, transform_a);
				aabbs.push_back(a_aabb);
			}
			else
			{
				a_aabb = aabbs[a_id];
			}

			int b_id = -1;
			for (auto body_b : query)
			{
				b_id++;

				collision_pair pair = { a_id, b_id };
				bool stop = false;
				for (auto old_pair : pairs)
				{
					if (old_pair == pair)
					{
						stop = true;
						break;
					}
				}

				if (stop)
				{
					continue;
				}


				if (body_a.region.get_id() == body_b.region.get_id())
				{
					continue;
				}

				pipo::transform_data& transform_b = body_b.get<pipo::transform_data>();

				if (transform_b.parent != peetcs::invalid_entity_id)
				{
					continue;
				}

				box_collider_data& collider_b = body_a.get<box_collider_data>();
				aabb b_aabb;
				if (aabbs.size() <= b_id)
				{
					b_aabb = get_collider_aabb(pool, collider_b, transform_b);
					aabbs.push_back(b_aabb);
				}
				else
				{
					b_aabb = aabbs[b_id];
				}

				if (!a_aabb.overlap(b_aabb))
				{
					continue;
				}

				pairs.push_back(pair);
				//gpu_context.draw_line_gizmo(a_aabb.min, a_aabb.max, { 0,1,0 });
			}

		}
	}
}
