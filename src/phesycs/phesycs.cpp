#include "include/phesycs/phesycs.hpp"
#include "include/phesycs/phesycs.hpp"

#include <glm/gtx/quaternion.hpp>

#include "tests/shared.hpp"

// ======================= AABB =============================


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
	pipo::transform_data collider_transform = collider_a.transform;

	glm::mat4 translation = glm::translate(glm::mat4(1.0f), transform_a.get_pos());
	glm::mat4 scale_mat = glm::scale(glm::mat4(1.0f), transform_a.get_scale());

	glm::mat4 ws_matrix = translation * scale_mat;

	aabb a_aabb = {};
	auto collider_matrix = ws_matrix * collider_transform.get_local_space();
	a_aabb.min = glm::vec3(collider_matrix * glm::vec4(collider_transform.get_pos() + collider_a.transform.get_scale() * -1.f, 1.f) );
	a_aabb.max = glm::vec3(collider_matrix * glm::vec4(collider_transform.get_pos() + collider_a.transform.get_scale() * 1.f, 1.f));

	if (a_aabb.min.x > a_aabb.max.x) std::swap(a_aabb.min.x, a_aabb.max.x);
	if (a_aabb.min.y > a_aabb.max.y) std::swap(a_aabb.min.y, a_aabb.max.y);
	if (a_aabb.min.z > a_aabb.max.z) std::swap(a_aabb.min.z, a_aabb.max.z);

	return a_aabb;
}

// ======================= GJK =============================

struct support
{
	glm::vec3 c;
	glm::vec3 a;
	glm::vec3 b;
};

struct simplex
{
	std::array<support, 4> points;
	int size;

	void add(const support& point)
	{
		if (size >= 4)
		{
			return;
		}

		points[size] = point;
		size++;
	}
};

static inline glm::vec3 furthest_point_in_direction(const std::vector<glm::vec3>& shape, glm::vec3 dir)
{
	float max_dot = -INFINITY;
	glm::vec3 max_point = { 0,0,0 };

	for (const auto& point : shape)
	{
		float dot = glm::dot(point, dir);
		if (dot > max_dot)
		{
			max_dot = dot;
			max_point = point;
		}
	}
	return max_point;
}

static inline support support_full(const std::vector<glm::vec3>& a, const std::vector<glm::vec3>& b, glm::vec3& dir)
{
	glm::vec3 point_a = furthest_point_in_direction(a, dir);
	glm::vec3 point_b = furthest_point_in_direction(b, -dir);

	return support{ point_a - point_b, point_a, point_b };
}

static inline bool  same_direction(const glm::vec3& a, const glm::vec3& b)
{
	return glm::dot(a, b) > 0;
}

static inline glm::vec3 get_origin_dir(glm::vec3& a, glm::vec3& b)
{
	glm::vec3 ab = b - a;
	glm::vec3 ao = -a;
	return glm::cross(glm::cross(ab, ao), ab);
}

static inline bool handle_simplex(simplex& simplex, glm::vec3& direction) {
	if (simplex.size == 2) 
	{
		direction = get_origin_dir(simplex.points[1].c, simplex.points[0].c);
		return false;
	}
	else if (simplex.size == 3) 
	{
		auto a = simplex.points[2].c;
		auto b = simplex.points[1].c;
		auto c = simplex.points[0].c;

		auto ao = -a;

		auto abp = get_origin_dir(a, b);
		if (!same_direction(abp, ao))
		{
			simplex.points = { b, a };
			simplex.size = 2;
			direction = abp;
			return false;
		}

		auto acp = get_origin_dir(a, c);
		if (!same_direction(acp, ao))
		{
			simplex.points = { c, a };
			simplex.size = 2;
			direction = acp;
			return false;
		}

		glm::vec<3, float> normal;
		auto ab = b - a;
		auto ac = c - a;
		normal = glm::cross(ab, ac);

		if (same_direction(normal, ao))
		{
			direction = normal;
		}
		else
		{
			direction = -normal;
			std::swap(simplex.points[0], simplex.points[1]); // maintain winding
		}
		return false;
	}
	else if (simplex.size == 4) 
	{
		auto a = simplex.points[3].c;
		auto b = simplex.points[2].c;
		auto c = simplex.points[1].c;
		auto d = simplex.points[0].c;

		auto ao = -a;

		auto ab = b - a;
		auto ac = c - a;
		auto ad = d - a;

		auto abc = glm::cross(ab, ac);
		if (glm::dot(abc, ao) > 0) abc = -abc;

		auto acd = glm::cross(ac, ad);
		if (glm::dot(acd, ao) > 0) acd = -acd;

		auto adb = glm::cross(ad, ab);
		if (glm::dot(adb, ao) > 0) adb = -adb;

		if (same_direction(abc, ao)) 
		{
			simplex = { simplex.points[3], simplex.points[2], simplex.points[1] }; // a, b, c
			simplex.size = 3;
			direction = abc;
			return false;
		}

		if (same_direction(acd, ao)) 
		{
			simplex = { simplex.points[3], simplex.points[1], simplex.points[0] }; // a, c, d
			simplex.size = 3;
			direction = acd;
			return false;
		}
		if (same_direction(adb, ao)) 
		{
			simplex = { simplex.points[3], simplex.points[0], simplex.points[2] }; // a, d, b
			simplex.size = 3;
			direction = adb;
			return false;
		}

		return true;
	}

	return false;
}

static bool gjk(const std::vector<glm::vec3>& a, const std::vector<glm::vec3>& b, simplex& simplex)
{
	glm::vec3 dir = glm::vec3(1, 0, 0);
	simplex.add(support_full(a, b, dir));

	dir = -simplex.points[0].c;

	for (int i =0; i < 5; i++) 
	{
		dir = glm::normalize(dir);

		support new_pt = support_full(a, b, dir);
		if (glm::dot(new_pt.c, dir) < 0)
			return false;

		simplex.add(new_pt);

		if (handle_simplex(simplex, dir))
			return true;
	}

	return false;
}

// ======================= EPA =============================

struct face {
	int a, b, c;
	glm::vec3 normal;
	float distance;
};

struct edge {
	int a, b;
	bool operator==(const edge& e) const { return (a == e.b && b == e.a) || (a == e.a && b == e.b); }
};


struct collision_test
{
	bool collision;
	glm::vec3 axis;
	float resolution;
	glm::vec3 position;
};

face make_face(int a, int b, int c, const std::vector<support>& polytope)
{
	glm::vec3 normal = glm::normalize(glm::cross(polytope[b].c - polytope[a].c, polytope[c].c - polytope[a].c));
	float distance = glm::dot(normal, polytope[a].c);
	return face{ a, b, c, normal, distance };
}

void add_edge(int a, int b, std::vector<edge>&edge_list)
{
	edge e{ a, b };
	auto it = std::find(edge_list.begin(), edge_list.end(), e);
	if (it != edge_list.end())
	{
		// Edge already exists (reverse direction) → remove it
		edge_list.erase(it);
	}
	else
	{
		// New edge → add it
		edge_list.push_back(e);
	}
};

collision_test epa(const std::vector<glm::vec3>& a, const std::vector<glm::vec3>& b, simplex& simplex)
{
	collision_test result;

	// TODO use something more cache efficient than a vector (as well for the edge list)
	std::vector<support> polytope;
	std::vector<face> faces;

	for (int i = 0; i < simplex.size; ++i) {
		polytope.push_back(simplex.points[i]);
	}

	// Initialize tetrahedron faces
	faces.push_back(make_face(0, 1, 2, polytope));
	faces.push_back(make_face(0, 2, 3, polytope));
	faces.push_back(make_face(0, 3, 1, polytope));
	faces.push_back(make_face(1, 3, 2, polytope));

	// hardcoded out but there should be a mathematical way to approach this.
	for (int attempts = 0; attempts < 30; attempts++)
	{
		// Find face closest to origin along normal
		int closest = -1;
		float min_dist = FLT_MAX;
		for (int i = 0; i < (int)faces.size(); ++i) 
		{
			// NAN check
			if (faces[i].distance != faces[i].distance)
			{
				continue;
			}

			if (faces[i].distance < min_dist)
			{
				min_dist = faces[i].distance;
				closest = i;
			}
		}

		face& f = faces[closest];
		support p = support_full(a, b, f.normal);

		// TODO convert these to barycentric coordinates instead of poly coordinates
		float d = glm::dot(p.c, f.normal);
		if (d - f.distance < 0.001f) 
		{
			// Converged
			glm::vec3 contactA = p.a;
			glm::vec3 contactB = p.b;

			result.collision = true;
			result.position = contactA - f.normal * (d * 0.5f);
			result.axis = f.normal;
			result.resolution = d;

			return result;
		}

		std::vector<edge> edge_list;

		// Remove faces visible from p and collect their edges
		for (int i = 0; i < (int)faces.size(); ) 
		{
			if (glm::dot(faces[i].normal, p.c - polytope[faces[i].a].c) > 0) 
			{
				add_edge(faces[i].a, faces[i].b, edge_list);
				add_edge(faces[i].b, faces[i].c, edge_list);
				add_edge(faces[i].c, faces[i].a, edge_list);
				faces.erase(faces.begin() + i);
			}
			else 
			{
				++i;
			}
		}

		polytope.push_back(p);
		int idx = (int)polytope.size() - 1;

		// Rebuild polytope with new faces from border edges
		for (auto& e : edge_list) 
		{
			faces.push_back(make_face(e.a, e.b, idx, polytope));
		}
	}

	result.collision = false;
	return result;
}


void resolve_3d_collision(phesycs_impl::rigid_body_data& a, phesycs_impl::rigid_body_data& b, const collision_test& collision)
{
	float elasticity = 0.5;
	float friction = 0.0f;

	glm::vec3 ra = collision.position - phesycs_impl::to_vec3(a.transform.position);
	glm::vec3 rb = collision.position - phesycs_impl::to_vec3(b.transform.position);

	glm::vec3 va = phesycs_impl::to_vec3(a.velocity) + cross(phesycs_impl::to_vec3(a.angular_velocity), ra);
	glm::vec3 vb = phesycs_impl::to_vec3(b.velocity) + cross(phesycs_impl::to_vec3(b.angular_velocity), rb);

	glm::vec3 relative_velocity = vb - va;

	glm::vec3 normal = glm::normalize(collision.axis);

	float vel_along_normal = glm::dot(relative_velocity, normal);

	// If they are separating, no impulse needed
	if (vel_along_normal > 0.0f)
		return;

	auto inv_tern_a = a.is_static ? glm::mat3(0.0f) : a.get_inverse_inertia();
	auto inv_tern_b = b.is_static ? glm::mat3(0.0f) : b.get_inverse_inertia();

	float denom = 
		(a.is_static ? 0.f : a.inverse_mass) + 
		(b.is_static ? 0.f : b.inverse_mass) + glm::dot(collision.axis,
			glm::cross(inv_tern_a * glm::cross(ra, normal), ra) +
			glm::cross(inv_tern_b * glm::cross(rb, normal), rb));

	// Compute the scalar impulse magnitude (j)
	float j = -(1.0f + elasticity) * vel_along_normal / denom;

	// Final impulse vector in world space
	glm::vec3 impulse = j * normal;

	if (!a.is_static)
	{
		apply_linear_impulse_api(a, -impulse);
		apply_angular_impulse_api(a, -impulse, ra);
	}

	if (!b.is_static)
	{
		apply_linear_impulse_api(b, impulse);
		apply_angular_impulse_api(b, impulse, rb);
	}

	// Friction
	//float a_friction = friction * (1.f - abs(glm::dot(glm::normalize(impulse), glm::normalize(a.get_velocity()))));
	a.set_translational_velocity(a.get_velocity() * (1.f - friction));
	//a.set_angular_velocity(a.get_angular_velocity() * (1.f - friction));

	//float b_friction = friction * (1.f - abs(glm::dot(glm::normalize(impulse), glm::normalize(b.get_velocity()))));
	b.set_translational_velocity(b.get_velocity() * (1.f - friction));
	//b.set_angular_velocity(b.get_angular_velocity() * (1.f - friction));
}


void phesycs_impl::tick(peetcs::archetype_pool& pool, pipo& gpu_context)
{
	static time_info time;

	time.tick();
	time.delta_time *= 1.f;

	auto query = pool.query<pipo::transform_data, box_collider_data, rigid_body_data>();

	// tick new position and rotation
	for (auto value : query)
	{
		auto& transform = value.get<pipo::transform_data>();
		auto& rigid_body = value.get<rigid_body_data>();
		auto& collider = value.get<box_collider_data>();

		rigid_body.transform.set_pos(transform.get_pos());
		rigid_body.transform.set_rotation(transform.get_rotation());
		rigid_body.transform.set_pos(rigid_body.transform.get_pos() + rigid_body.get_velocity() * (float)time.delta_time);

		// 2. Convert angular velocity to a quaternion delta
		glm::vec3 angular_velocity = rigid_body.get_angular_velocity();
		float angle = glm::length(rigid_body.get_angular_velocity()) * (float)time.delta_time;

		if (angle > 0.0f) {
			glm::vec3 axis = glm::normalize(angular_velocity);
			glm::quat deltaRotation = glm::angleAxis(angle, axis);

			// 3. Apply rotation
			rigid_body.transform.set_rotation(glm::normalize(deltaRotation * rigid_body.transform.get_rotation())); // Apply rotation in world space
		}

		// dampening
		rigid_body.set_translational_velocity(rigid_body.get_velocity() * 0.999f);
		rigid_body.set_angular_velocity(rigid_body.get_angular_velocity() * 0.999f);

		transform.set_pos(rigid_body.transform.get_pos());
		transform.set_rotation(rigid_body.transform.get_rotation());

		if (!rigid_body.is_static)
		{
			rigid_body.set_translational_velocity(rigid_body.get_velocity() + glm::vec3(0, -9.81f * (float)time.delta_time, 0));
		}
	}

	struct collision_pair
	{
		int a_id;
		int b_id;

		inline bool operator==(const collision_pair& other) const
		{
			return a_id == other.b_id && b_id == other.a_id || a_id == other.a_id && b_id == other.b_id;
		}
	};
	std::vector<aabb> aabbs = {};
	std::vector<collision_pair> pairs = {};

	int a_id = -1;
	for (auto body_a : query)
	{
		box_collider_data collider_a = body_a.get<box_collider_data>();
		pipo::transform_data& transform_a = body_a.get<pipo::transform_data>();

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

				box_collider_data& collider_b = body_b.get<box_collider_data>();
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

				// TODO these can be arrays
				std::vector<glm::vec3> a_vertices = pipo::primitives::cube::vertices;
				std::vector<glm::vec3> b_vertices = pipo::primitives::cube::vertices;

				std::vector<glm::vec3> a_axes = { {0,0,1}, {0,1,0}, {1,0,0} };
				std::vector<glm::vec3> b_axes = { {0,0,1}, {0,1,0}, {1,0,0} };

				auto a_ws_matrix = collider_a.transform.get_local_space() * transform_a.get_world_space(pool) ;
				auto b_ws_matrix = collider_b.transform.get_local_space()*  transform_b.get_world_space(pool) ;

				// Assume shape that the shape is a cube for now
				for (auto& a_vertex : a_vertices)
				{
					a_vertex =  a_ws_matrix * glm::vec4(a_vertex, 1);
				}

				for (auto& b_vertex : b_vertices)
				{
					b_vertex = b_ws_matrix * glm::vec4(b_vertex, 1);
				}

				auto a_rot_matrix = glm::toMat4(transform_a.get_rotation());
				auto b_rot_matrix = glm::toMat4(transform_b.get_rotation());

				for (auto& a_axis : a_axes)
				{
					a_axis = a_rot_matrix * glm::vec4(a_axis, 1);
				}

				for (auto& b_axis : b_axes)
				{
					b_axis = b_rot_matrix * glm::vec4(b_axis, 1);
				}

				collision_test test = {};
				simplex in_simplex = {};
				if (gjk(a_vertices, b_vertices, in_simplex))
				{
					test = epa(a_vertices, b_vertices, in_simplex);
				}

				if (test.collision)
				{
					float resolve_amount = 0.04f;
					//gpu_context.draw_cube_gizmo(test.position, { 0.01, 0.01, 0.01 }, { 0,0,0,1}, { 1,0,0 });
					//gpu_context.draw_line_gizmo(a_aabb.min, a_aabb.max, { 1,1,0 });


					rigid_body_data& a_rigidbody = body_a.get<rigid_body_data>();
					rigid_body_data& b_rigidbody = body_b.get<rigid_body_data>();

					if (a_rigidbody.is_static)
						gpu_context.draw_cube_gizmo(transform_a.get_pos(), transform_a.get_scale() * collider_a.transform.get_scale(), transform_a.get_rotation(), { 0, 1, 0 });

					if (b_rigidbody.is_static)
						gpu_context.draw_cube_gizmo(transform_b.get_pos(), transform_b.get_scale() * collider_b.transform.get_scale(), transform_b.get_rotation(), { 0, 1, 0 });


					if (!a_rigidbody.is_static && !b_rigidbody.is_static)
					{
						transform_a.set_pos(transform_a.get_pos() + test.axis * test.resolution * -resolve_amount);
						transform_b.set_pos(transform_b.get_pos() + test.axis * test.resolution * resolve_amount);
					}
					else if (!a_rigidbody.is_static)
					{
						transform_a.set_pos(transform_a.get_pos() + test.axis * test.resolution * -resolve_amount);
					}
					else if (!b_rigidbody.is_static)
					{
						transform_b.set_pos(transform_b.get_pos() + test.axis * test.resolution * resolve_amount);
					}

					a_aabb = get_collider_aabb(pool, collider_a, transform_a);
					aabbs[a_id] = a_aabb;
					resolve_3d_collision(a_rigidbody, b_rigidbody, test);
				}
			}
		}
	}

}
