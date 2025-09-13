
#include "include/phesycs/phesycs.hpp"
#include "include/phesycs/phesycs.hpp"
#include <algorithm>
#include <execution>
#include <unordered_set>
#include <glm/gtx/quaternion.hpp>

#include "tests/shared.hpp"

#undef min
#undef max

static bool debug_visuals = false;

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

		points = { point, points[0], points[1], points[2] };
		size++;
	}
};


template<typename It>
static inline glm::vec3 furthest_point_in_direction(const It& shape, glm::vec3 dir)
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

template<typename ItA, typename ItB>
static inline support support_full(const ItA& a, const ItB& b, glm::vec3& dir)
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

static glm::vec3 triple_product(const glm::vec3& a, glm::vec3 b, glm::vec3 c)
{
	return glm::cross(glm::cross(a, b), c);
}

static bool handle_line(simplex& simplex, glm::vec3& direction)
{
	auto a = simplex.points[0].c;
	auto b = simplex.points[1].c;

	auto ab = b - a;
	auto ao = -a;

	if (glm::dot(ab, ao) > 0)
	{
		direction = triple_product(ab, ao, ab);
	}
	else
	{
		simplex = { simplex.points[0] };
		direction = ao;
	}

	return false;
}

static bool handle_triangle(simplex& simplex, glm::vec3& direction)
{
	auto a = simplex.points[0].c;
	auto b = simplex.points[1].c;
	auto c = simplex.points[2].c;

	auto ab = b - a;
	auto ac = c - a;
	auto ao = -a;

	// Get triangle normal
	glm::vec<3, float> abc = glm::cross(ab, ac);
	if (glm::dot(glm::cross(abc, ac), ao) > 0)
	{
		if (glm::dot(ac, ao) > 0)
		{
			simplex = { simplex.points[0], simplex.points[2] }; // a - c
			simplex.size = 2;
			direction = triple_product(ac, ao, ac);
		}
		else
		{
			simplex = { simplex.points[0], simplex.points[1] }; // a - b
			simplex.size = 2;
			return handle_line(simplex, direction);
		}
	}
	else
	{
		if (glm::dot(glm::cross(ab, abc), ao) > 0)
		{
			simplex = { simplex.points[0], simplex.points[1] }; // a - b
			simplex.size = 2;
			return handle_line(simplex, direction);
		}

		if (glm::dot(abc, ao) > 0)
		{
			direction = abc;
		}
		else
		{
			simplex = { simplex.points[0], simplex.points[2], simplex.points[1] }; // a - c - b
			simplex.size = 3;
			direction = -abc;
		}
	}

	return false;
}



static inline bool handle_simplex(simplex& simplex, glm::vec3& direction)
{
	if (simplex.size == 2)
	{
		return handle_line(simplex, direction);
	}
	else if (simplex.size == 3)
	{
		return handle_triangle(simplex, direction);
	}
	else if (simplex.size == 4)
	{
		auto a = simplex.points[0].c;
		auto b = simplex.points[1].c;
		auto c = simplex.points[2].c;
		auto d = simplex.points[3].c;

		auto ao = -a;
		auto ab = b - a;
		auto ac = c - a;
		auto ad = d - a;

		auto abc = glm::cross(ab, ac);
		auto acd = glm::cross(ac, ad);
		auto adb = glm::cross(ad, ab);

		if (same_direction(abc, ao))
		{
			simplex = { simplex.points[0], simplex.points[1], simplex.points[2] }; // a, b, c
			simplex.size = 3;
			return handle_triangle(simplex, direction);
		}

		if (same_direction(acd, ao))
		{
			simplex = { simplex.points[0], simplex.points[2], simplex.points[3] }; // a, c, d
			simplex.size = 3;
			return handle_triangle(simplex, direction);
		}
		if (same_direction(adb, ao))
		{
			simplex = { simplex.points[0], simplex.points[3], simplex.points[1] }; // a, d, b
			simplex.size = 3;
			return handle_triangle(simplex, direction);
		}

		return true;
	}

	return false;
}


// Based on: https://winter.dev/articles/gjk-algorithm
template<typename ItA, typename ItB>
static bool gjk(const ItA& a, const ItB& b, simplex& simplex)
{
	glm::vec3 dir = glm::vec3(1, 0, 0);
	simplex.add(support_full(a, b, dir));

	dir = -simplex.points[0].c;

	for (int i = 0; i < 10; i++)
	{
		dir = glm::normalize(dir);

		support new_pt = support_full(a, b, dir);
		if (glm::dot(new_pt.c, dir) <= 0)
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
	glm::vec3 ab = polytope[b].c - polytope[a].c;
	glm::vec3 ac = polytope[c].c - polytope[a].c;

	glm::vec3 normal = glm::normalize(glm::cross(ab, ac));

	// Ensure normal points outward (away from origin)
	if (glm::dot(normal, polytope[a].c) < 0.0f)
		normal = -normal;

	float distance = glm::dot(normal, polytope[a].c);

	return face{ a, b, c, normal, distance };
}

void add_edge(int a, int b, std::vector<edge>& edge_list)
{
	edge e{ a, b };
	auto it = std::find(edge_list.begin(), edge_list.end(), e);
	if (it != edge_list.end())
	{
		// Edge already exists (reverse direction): remove it
		edge_list.erase(it);
	}
	else
	{
		// New edge: add it
		edge_list.push_back(e);
	}
};

struct ContactPoints {
	glm::vec3 contactA;
	glm::vec3 contactB;
};

static glm::vec3 closest_point_on_triangle_to_origin(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
	// Edge vectors
	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ap = -a;

	float d1 = glm::dot(ab, ap);
	float d2 = glm::dot(ac, ap);

	if (d1 <= 0.0f && d2 <= 0.0f) return a; // Closest to a

	glm::vec3 bp = -b;
	float d3 = glm::dot(ab, bp);
	float d4 = glm::dot(ac, bp);
	if (d3 >= 0.0f && d4 <= d3) return b; // Closest to b

	float vc = d1 * d4 - d3 * d2;
	if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
	{
		float v = d1 / (d1 - d3);
		return a + v * ab; // Closest to ab edge
	}

	glm::vec3 cp = -c;
	float d5 = glm::dot(ab, cp);
	float d6 = glm::dot(ac, cp);
	if (d6 >= 0.0f && d5 <= d6) return c; // Closest to c

	float vb = d5 * d2 - d1 * d6;
	if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
	{
		float w = d2 / (d2 - d6);
		return a + w * ac; // Closest to ac edge
	}

	float va = d3 * d6 - d5 * d4;
	if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
	{
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		return b + w * (c - b); // Closest to bc edge
	}

	// Inside face
	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	return a + ab * v + ac * w;
}


glm::vec3 compute_worldspace_collision(const support& A, const support& B, const support& C)
{
	// Step 1: Project origin onto the triangle
	glm::vec3 p = closest_point_on_triangle_to_origin(A.c, B.c, C.c);

	// Step 2: Compute barycentric coordinates of p w.r.t. triangle A.c, B.c, C.c
	glm::vec3 v0 = B.c - A.c;
	glm::vec3 v1 = C.c - A.c;
	glm::vec3 v2 = p - A.c;

	float d00 = glm::dot(v0, v0);
	float d01 = glm::dot(v0, v1);
	float d11 = glm::dot(v1, v1);
	float d20 = glm::dot(v2, v0);
	float d21 = glm::dot(v2, v1);
	float denom = d00 * d11 - d01 * d01;

	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	float u = 1.0f - v - w;

	// Step 3: Interpolate original world-space support points
	glm::vec3 contactA = u * A.a + v * B.a + w * C.a;
	glm::vec3 contactB = u * A.b + v * B.b + w * C.b;

	return contactA;
}

// Based on: https://allenchou.net/2013/12/game-physics-contact-generation-epa/
template<int ASize, int BSize, typename ItA, typename ItB>
collision_test epa(const ItA& a, const ItB& b, simplex& simplex)
{
	collision_test result = {};

	// TODO use something more cache efficient than a vector (as well for the edge list)
	std::vector<support> polytope;
	std::vector<face> faces;

	polytope.reserve(ASize * BSize);
	faces.reserve(ASize * BSize);

	for (int i = 0; i < simplex.size; ++i) {
		polytope.push_back(simplex.points[i]);
	}

	// Initialize tetrahedron faces
	faces.push_back(make_face(0, 1, 2, polytope));
	faces.push_back(make_face(0, 2, 3, polytope));
	faces.push_back(make_face(0, 3, 1, polytope));
	faces.push_back(make_face(1, 3, 2, polytope));

	int max_attemps = ASize * BSize;

	// hardcoded out but there should be a mathematical way to approach this.
	for (int attempts = 0; attempts <= max_attemps; attempts++)
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

		if (closest == -1)
		{
			return {};
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
			result.position = compute_worldspace_collision(polytope[f.a], polytope[f.b], polytope[f.c]);
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

	return result;
}

// Source: https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics6collisionresponse/2017%20Tutorial%206%20-%20Collision%20Response.pdf
void resolve_3d_collision(phesycs_impl::rigid_body_data& a, phesycs_impl::rigid_body_data& b, const collision_test& collision)
{
	
	if (glm::length(collision.axis) == 0.f)
	{
		return;
	}

	float elasticity = 0.0;

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

	auto inv_tern_a = a.is_static ? glm::mat3(0) : a.get_inverse_inertia();
	auto inv_tern_b = b.is_static ? glm::mat3(0) : b.get_inverse_inertia();

	float denom =
		(a.is_static ? 0.f : a.inverse_mass) +
		(b.is_static ? 0.f : b.inverse_mass) + glm::dot(collision.axis,
			glm::cross(inv_tern_a * glm::cross(ra, normal), ra) +
			glm::cross(inv_tern_b * glm::cross(rb, normal), rb));

	// Compute the scalar impulse magnitude (j)
	float j = -(1.0f + elasticity) * vel_along_normal / denom;

	// Final impulse vector in world space
	glm::vec3 impulse = j * collision.axis;

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

	va = phesycs_impl::to_vec3(a.velocity) + cross(phesycs_impl::to_vec3(a.angular_velocity), ra);
	vb = phesycs_impl::to_vec3(b.velocity) + cross(phesycs_impl::to_vec3(b.angular_velocity), rb);

	relative_velocity = vb - va;
	vel_along_normal = glm::dot(relative_velocity, normal);

	// Friction
	glm::vec3 tangent = relative_velocity - collision.axis * vel_along_normal;
	float tangent_length = glm::length(tangent);
	if (tangent_length > 1e-6f)
	{
		tangent = tangent / tangent_length;
		float frictional_mass = 
			(a.is_static ? 0.f : a.inverse_mass) +
			(b.is_static ? 0.f : b.inverse_mass) + glm::dot(tangent,
				glm::cross(inv_tern_a * glm::cross(ra, tangent), ra) +
				glm::cross(inv_tern_b * glm::cross(rb, tangent), rb));

		if (frictional_mass > 0.f)
		{
			float friction_coeff = a.friction * b.friction;
			float jt = (- glm::dot(relative_velocity, tangent) * friction_coeff) / frictional_mass;


			if (!a.is_static)
			{
				a.set_translational_velocity(a.get_velocity() - tangent * (jt * a.get_inverse_inertia()));
				a.set_angular_velocity(a.get_angular_velocity() - a.get_inverse_inertia() * glm::cross(ra, tangent * jt));
			}

			if (!b.is_static)
			{
				b.set_translational_velocity(b.get_velocity() + tangent * (jt * b.get_inverse_inertia()));
				b.set_angular_velocity(b.get_angular_velocity() + b.get_inverse_inertia() * glm::cross(rb, tangent * jt));
			}

		}
	}
}

static void update_collider_data(peetcs::archetype_pool& pool, phesycs_impl::box_collider_data& collider, pipo::transform_data& transform)
{
	auto a_ws_matrix =  transform.get_world_space(pool) * collider.transform.get_local_space();
	collider.vertices = pipo::primitives::cube::vertices;

	collider.aabb.min = glm::vec3(INFINITY);
	collider.aabb.max = glm::vec3(-INFINITY);

	for (auto& a_vertex : collider.vertices)
	{
		a_vertex = a_ws_matrix * glm::vec4(a_vertex, 1);
		collider.aabb.max = glm::max(collider.aabb.max, a_vertex);
		collider.aabb.min = glm::min(collider.aabb.min, a_vertex);
	}
}

struct collision_pair {
	int a_id;
	int b_id;

	inline bool operator==(const collision_pair& other) const {
		return (a_id == other.a_id && b_id == other.b_id) ||
			(a_id == other.b_id && b_id == other.a_id);
	}
};

// Hash functor with symmetric hashing
struct collision_pair_hash {
	std::size_t operator()(const collision_pair& cp) const {
		// Compute symmetric hash (independent of order)
		int h1 = std::hash<int>{}(cp.a_id);
		int h2 = std::hash<int>{}(cp.b_id);
		// Combine in a way that's symmetric
		return h1 ^ h2;
	}
};

static bool is_pair_checked(std::unordered_set<collision_pair>& pairs, collision_pair pair)
{
	bool stop = false;


	return false;
}

void phesycs_impl::tick_collision_response(peetcs::archetype_pool& pool, pipo& gpu_context)
{
	auto query = pool.query<pipo::transform_data, box_collider_data, rigid_body_data>();
	
	std::unordered_set<collision_pair, collision_pair_hash> pairs = {};

	int max_id = -1;
	int a_id = -1;

	for (auto body_a : query)
	{
		box_collider_data* collider_a = &body_a.get<box_collider_data>();
		pipo::transform_data& transform_a = body_a.get<pipo::transform_data>();
		generic_container* collider_as = pool.get_list_container<box_collider_data>(body_a.get_id());
		int collider_list_index_a = 0;

		if (transform_a.parent == peetcs::invalid_entity_id)
		{
			a_id++;
			max_id = std::max(max_id, a_id);
			if (debug_visuals)
			{
				gpu_context.draw_vertices(std::vector(collider_a->vertices.begin(), collider_a->vertices.end()), pipo::primitives::cube::indices, { 1, 0, 0 });
			}

			// WARNING: Jump label
			jump_body_a:

			if (a_id == max_id)
			{
				update_collider_data(pool, *collider_a, transform_a);
			}

			int b_id = -1;
			for (auto body_b : query)
			{
				b_id++;
				max_id = std::max(max_id, b_id);

				// skip self check 
				if (a_id == b_id)
				{
					continue;
				}

				collision_pair pair = { a_id, b_id };
		
				if (pairs.contains(pair))
					continue;
				

				// Is not a child transform (not yet supported)
				pipo::transform_data& transform_b = body_b.get<pipo::transform_data>();
				if (transform_b.parent != peetcs::invalid_entity_id)
				{
					continue;
				}

				box_collider_data* collider_b = &body_b.get<box_collider_data>();

				generic_container* collider_bs = pool.get_list_container<box_collider_data>(body_b.get_id());
				int collider_list_index_b = 0;

				// WARNING: Jump label
				jump_body_b:

				if (b_id == max_id)
				{
					update_collider_data(pool, *collider_b, transform_b);
				}

				if (!collider_a->aabb.overlap(collider_b->aabb))
				{
					continue;
				}
				//gpu_context.draw_aabb(collider_a.aabb.min, collider_a.aabb.max, { 0, 1, 0 });

				collision_test test = {};
				simplex in_simplex = {};
				if (gjk(collider_a->vertices, collider_b->vertices, in_simplex))
				{
					test = epa<8,8>(collider_a->vertices, collider_b->vertices, in_simplex);
					test.collision = true;

					if (debug_visuals)
					{
						gpu_context.draw_cube_gizmo(test.position, { 0.1,0.1,0.1 }, glm::angleAxis(1.f, test.axis), { 1,0,0 });
					}

					pairs.emplace(pair);
				}

				if (test.collision)
				{
					float resolve_amount = 0.5f;

					rigid_body_data& a_rigidbody = body_a.get<rigid_body_data>();
					rigid_body_data& b_rigidbody = body_b.get<rigid_body_data>();

					if (debug_visuals)
					{
						//gpu_context.draw_aabb(a_aabb.min, a_aabb.max, { 0,0,1 });
					}

					if (!a_rigidbody.is_static && !b_rigidbody.is_static)
					{
						transform_a.set_pos(transform_a.get_pos() + test.axis * test.resolution * -resolve_amount);
						transform_b.set_pos(transform_b.get_pos() + test.axis * test.resolution * resolve_amount);
						update_collider_data(pool, *collider_a, transform_a);
						update_collider_data(pool, *collider_b, transform_b);
					}
					else if (!a_rigidbody.is_static)
					{
						transform_a.set_pos(transform_a.get_pos() + test.axis * test.resolution * -resolve_amount * 2.f);
						update_collider_data(pool, *collider_a, transform_a);
					}
					else if (!b_rigidbody.is_static)
					{
						transform_b.set_pos(transform_b.get_pos() + test.axis * test.resolution * resolve_amount * 2.f);
						update_collider_data(pool, *collider_b, transform_b);

					}

					resolve_3d_collision(a_rigidbody, b_rigidbody, test);
				}

				if (collider_bs && collider_list_index_b < collider_bs->size())
				{
					collider_b = &collider_bs->get_element_at(collider_list_index_b).get<box_collider_data>();
					collider_list_index_b++;

					// WARNING: Jump label
					goto jump_body_b;
				}
			}

			if (collider_as && collider_list_index_a < collider_as->size())
			{
				collider_a = &collider_as->get_element_at(collider_list_index_a).get<box_collider_data>();
				collider_list_index_a++;

				if (debug_visuals)
				{
					//gpu_context.draw_vertices(std::vector(collider_a->vertices.begin(), collider_a->vertices.end()), pipo::primitives::cube::indices, { 0, 1, 0 });
				}

				// WARNING: Jump label
				goto jump_body_a;
			}
		}
	}
}


void phesycs_impl::tick_spring_mass_integration(peetcs::archetype_pool& pool, pipo& gpu_context)
{
	static time_info time;

	time.tick();
	time.delta_time *= 1.0f;

	time.delta_time = std::min(1 / 60., time.delta_time);

	auto query = pool.query<pipo::transform_data, rigid_body_data, spring_mass_data>();
	for (auto query_value : query)
	{

		auto& spring_mass = query_value.get<spring_mass_data>();
		auto& transform_a = query_value.get<pipo::transform_data>();
		auto transform_b_ptr = pool.get_from_owner<pipo::transform_data>(spring_mass.b);
		if (!transform_b_ptr)
		{
			continue;
		}

		// Check if B is still valid
		auto* rigid_body_b_ptr = pool.get_from_owner<rigid_body_data>(spring_mass.b);
		if (!rigid_body_b_ptr)
		{
			continue;
		}

		rigid_body_data& rigid_body_a = query_value.get<rigid_body_data>();
		rigid_body_data& rigid_body_b = *rigid_body_b_ptr;

		glm::vec3 a = transform_a.get_ws_pos(pool);
		glm::vec3 b = transform_b_ptr->get_ws_pos(pool);
		gpu_context.draw_line_gizmo(a, b, { 00.1,00.1,0.3 });

		glm::vec3 ab = b - a;
		float ab_length = glm::length2(ab);
		float x = ab_length - spring_mass.rest_length;

		float force_stiffness = spring_mass.stiffness * x;

		glm::vec3 ab_vel = rigid_body_b.get_velocity() - rigid_body_a.get_velocity();

		glm::vec3 ab_normal = { -1.f, 0, 0 };
		if (glm::length(ab) != 0.f)
		{
			ab_normal = glm::normalize(ab);
		}

		if (glm::length(ab_normal) > 1.f)
		{
			continue;
		}

		float force_dampening = spring_mass.damping * glm::dot(ab_normal, ab_vel);

		float force_total = force_stiffness + force_dampening;
		glm::vec3 ba = a - b;

		glm::vec3 ba_normal = { 1, 0, 0 };
		if (glm::length(ba) != 0.f)
		{
			ba_normal = glm::normalize(ba);
		}

		if (glm::length(ba_normal) > 1.f)
		{
			continue;
		}

		if (!rigid_body_a.is_static)
		{
			glm::vec3 impulse_a = force_total * ab_normal * (float)time.get_delta_time();
			apply_linear_impulse_api(rigid_body_a, impulse_a);


			glm::vec3 forward = glm::cross(ab_normal, glm::cross(ab_normal, { 1,0,0 }));
			glm::quat lerped = glm::quatLookAt(forward, {0, 1, 0 });

			transform_a.set_rotation(glm::mix(transform_a.get_rotation(), lerped, glm::min(1.f, (float)time.get_delta_time() *2.f)));
		}

		if (!rigid_body_b.is_static)
		{
			glm::vec3 impulse_b = force_total * ba_normal * (float)time.get_delta_time();
			apply_linear_impulse_api(rigid_body_b, impulse_b);
			//apply_angular_impulse_api(rigid_body_b, impulse_b, ba);

			float t = 0.5f;     // interpolation factor [0..1]

			glm::quat lerped = glm::quatLookAt(ab_normal, { 0, 1, 0 });
			//transform_b_ptr->set_rotation(glm::mix(transform_b_ptr->get_rotation(), lerped, 0.5f));
			//transform_b_ptr->set_rotation(glm::mix(transform_b_ptr->get_rotation(), lerped, 0.5f));

		}
	}
}

void phesycs_impl::set_debug_visuals(bool enabled)
{
	debug_visuals = enabled;
}



void phesycs_impl::tick_integration(peetcs::archetype_pool& pool, pipo& gpu_context)
{
	static time_info time;

	time.tick();
	time.delta_time *= 1.0f;

	auto query = pool.query<pipo::transform_data, rigid_body_data>();

	// tick new position and rotation
	for (auto value : query)
	{
		auto& transform = value.get<pipo::transform_data>();
		auto& rigid_body = value.get<rigid_body_data>();

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
			rigid_body.transform.set_rotation(glm::normalize(deltaRotation * rigid_body.transform.get_rotation())); // Apply rotation in world 
		}

		// dampening
		rigid_body.set_translational_velocity(rigid_body.get_velocity() * 0.999f);
		rigid_body.set_angular_velocity(rigid_body.get_angular_velocity() * 0.999f);

		transform.set_pos(rigid_body.transform.get_pos());
		transform.set_rotation(rigid_body.transform.get_rotation());

		if (!rigid_body.is_static && rigid_body.gravity)
		{
			rigid_body.set_translational_velocity(rigid_body.get_velocity() + glm::vec3(0, -9.81f * (float)time.delta_time, 0));
		}
	}
}
