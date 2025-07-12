#include "tests/shared.hpp"

#include <iostream>

#include "include/archetype_pool.hpp"
#include "include/phesycs/phesycs.hpp"
#include "include/pipo/rasterizer.hpp"


void tick_integration_api(peetcs::archetype_pool& c_pool, pipo& gpu_context)
{
	phesycs_impl::tick_integration(c_pool, gpu_context);
}

void tick_spring_mass_integration_api(peetcs::archetype_pool& c_pool, pipo& gpu_context)
{
	phesycs_impl::tick_spring_mass_integration(c_pool, gpu_context);
}

void tick_collision_response_api(peetcs::archetype_pool& c_pool, pipo& gpu_context)
{
	phesycs_impl::tick_collision_response(c_pool, gpu_context);
}

// Apply a linear impulse directly to the center of mass
void apply_linear_impulse_api(phesycs_impl::rigid_body_data& body, const glm::vec3& impulse)
{
	body.set_translational_velocity(body.get_velocity() + impulse * body.inverse_mass);
}

// Apply an angular impulse using the cross product of the contact vector and impulse
void apply_angular_impulse_api(phesycs_impl::rigid_body_data& body, const glm::vec3& impulse, const glm::vec3& contact_vector)
{
	glm::vec3 torque = glm::cross(contact_vector, impulse); // Torque = r × F
	glm::vec3 angular_acceleration = body.get_inverse_inertia() * torque;
	body.set_angular_velocity(body.get_angular_velocity() + angular_acceleration);
}

