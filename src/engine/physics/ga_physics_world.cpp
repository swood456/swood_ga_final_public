/*
** RPI Game Architecture Engine
**
** Portions adapted from:
** Viper Engine - Copyright (C) 2016 Velan Studios - All Rights Reserved
**
** This file is distributed under the MIT License. See LICENSE.txt.
*/

#include "ga_physics_world.h"
#include "ga_intersection.h"
#include "ga_rigid_body.h"
#include "ga_shape.h"

#include "framework/ga_drawcall.h"
#include "framework/ga_frame_params.h"

#include <algorithm>
#include <assert.h>
#include <ctime>

typedef bool (*intersection_func_t)(const ga_shape* a, const ga_mat4f& transform_a, const ga_shape* b, const ga_mat4f& transform_b, ga_collision_info* info);

static intersection_func_t k_dispatch_table[k_shape_count][k_shape_count];

ga_physics_world::ga_physics_world()
{
	// Clear the dispatch table.
	for (int i = 0; i < k_shape_count; ++i)
	{
		for (int j = 0; j < k_shape_count; ++j)
		{
			k_dispatch_table[i][j] = intersection_unimplemented;
		}
	}

	k_dispatch_table[k_shape_oobb][k_shape_oobb] = separating_axis_test;
	k_dispatch_table[k_shape_plane][k_shape_oobb] = oobb_vs_plane;
	k_dispatch_table[k_shape_oobb][k_shape_plane] = oobb_vs_plane;

	// Default gravity to Earth's constant.
	_gravity = { 0.0f, -9.807f, 0.0f };
}

ga_physics_world::~ga_physics_world()
{
	assert(_bodies.size() == 0);
}

void ga_physics_world::add_rigid_body(ga_rigid_body* body)
{
	while (_bodies_lock.test_and_set(std::memory_order_acquire)) {}
	_bodies.push_back(body);
	_bodies_lock.clear(std::memory_order_release);
}

void ga_physics_world::remove_rigid_body(ga_rigid_body* body)
{
	while (_bodies_lock.test_and_set(std::memory_order_acquire)) {}
	_bodies.erase(std::remove(_bodies.begin(), _bodies.end(), body));
	_bodies_lock.clear(std::memory_order_release);
}

void ga_physics_world::step(ga_frame_params* params)
{
	while (_bodies_lock.test_and_set(std::memory_order_acquire)) {}

	// Step the physics sim.
	for (int i = 0; i < _bodies.size(); ++i)
	{
		if (_bodies[i]->_flags & k_static) continue;

		ga_rigid_body* body = _bodies[i];

		if ((_bodies[i]->_flags & k_weightless) == 0)
		{
			body->_forces.push_back(_gravity);
		}

		step_linear_dynamics(params, body);
		step_angular_dynamics(params, body);
	}

	test_intersections(params);

	_bodies_lock.clear(std::memory_order_release);
}

void ga_physics_world::test_intersections(ga_frame_params* params)
{
	// Intersection tests. Naive N^2 comparisons.
	for (int i = 0; i < _bodies.size(); ++i)
	{
		for (int j = i + 1; j < _bodies.size(); ++j)
		{
			ga_shape* shape_a = _bodies[i]->_shape;
			ga_shape* shape_b = _bodies[j]->_shape;
			intersection_func_t func = k_dispatch_table[shape_a->get_type()][shape_b->get_type()];

			ga_collision_info info;
			bool collision = func(shape_a, _bodies[i]->_transform, shape_b, _bodies[j]->_transform, &info);
			if (collision)
			{
				std::time_t time = std::chrono::system_clock::to_time_t(
					std::chrono::system_clock::now());

#if defined(GA_PHYSICS_DEBUG_DRAW)
				ga_dynamic_drawcall collision_draw;
				collision_draw._positions.push_back(ga_vec3f::zero_vector());
				collision_draw._positions.push_back(info._normal);
				collision_draw._indices.push_back(0);
				collision_draw._indices.push_back(1);
				collision_draw._color = { 1.0f, 1.0f, 0.0f };
				collision_draw._draw_mode = GL_LINES;
				collision_draw._material = nullptr;
				collision_draw._transform.make_translation(info._point);

				while (params->_dynamic_drawcall_lock.test_and_set(std::memory_order_acquire)) {}
				params->_dynamic_drawcalls.push_back(collision_draw);
				params->_dynamic_drawcall_lock.clear(std::memory_order_release);
#endif
				// We should not attempt to resolve collisions if we're paused and have not single stepped.
				bool should_resolve = params->_delta_time > std::chrono::milliseconds(0) || params->_single_step;

				if (should_resolve)
				{
					resolve_collision(_bodies[i], _bodies[j], &info);
				}
			}
		}
	}
}

void ga_physics_world::step_linear_dynamics(ga_frame_params* params, ga_rigid_body* body)
{
	// Linear dynamics.
	ga_vec3f overall_force = ga_vec3f::zero_vector();
	while (body->_forces.size() > 0)
	{
		overall_force += body->_forces.back();
		body->_forces.pop_back();
	}

	float delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(params->_delta_time).count();

	ga_vec3f v1 = body->_velocity;
	ga_vec3f a1 = overall_force.scale_result(1.0f / body->_mass);

	ga_vec3f v2 = body->_velocity + a1.scale_result(0.5f * delta_time);
	ga_vec3f a2 = overall_force.scale_result(1.0f / body->_mass);

	ga_vec3f v3 = body->_velocity + a2.scale_result(0.5f * delta_time);
	ga_vec3f a3 = overall_force.scale_result(1.0f / body->_mass);

	ga_vec3f v4 = body->_velocity + a3.scale_result(delta_time);
	ga_vec3f a4 = overall_force.scale_result(1.0f / body->_mass);

	body->_transform.set_translation( body->_transform.get_translation() + (v1 + v2.scale_result(2.0f) + v3.scale_result(2.0f) + v4).scale_result(delta_time / 6.0f));
	body->_velocity = body->_velocity + (a1 + a2.scale_result(2.0f) + a3.scale_result(2.0f) + a4).scale_result(delta_time / 6.0f);
	
}

void ga_physics_world::step_angular_dynamics(ga_frame_params* params, ga_rigid_body* body)
{

}

void ga_physics_world::resolve_collision(ga_rigid_body* body_a, ga_rigid_body* body_b, ga_collision_info* info)
{
	// First move the objects so they no longer intersect.
	// Each object will be moved proportionally to their incoming velocities.
	// If an object is static, it won't be moved.
	float total_velocity = body_a->_velocity.mag() + body_b->_velocity.mag();
	float percentage_a = (body_a->_flags & k_static) ? 0.0f : body_a->_velocity.mag() / total_velocity;
	float percentage_b = (body_b->_flags & k_static) ? 0.0f : body_b->_velocity.mag() / total_velocity;

	// To avoid instability, nudge the two objects slightly farther apart.
	const float k_nudge = 0.001f;
	if ((body_a->_flags & k_static) == 0 && body_a->_velocity.mag2() > 0.0f)
	{
		float pen_a = info->_penetration * percentage_a + k_nudge;
		body_a->_transform.set_translation(body_a->_transform.get_translation() - body_a->_velocity.normal().scale_result(pen_a));
	}
	if ((body_b->_flags & k_static) == 0 && body_b->_velocity.mag2() > 0.0f)
	{
		float pen_b = info->_penetration * percentage_b + k_nudge;
		body_b->_transform.set_translation(body_b->_transform.get_translation() - body_b->_velocity.normal().scale_result(pen_b));
	}

	// Average the coefficients of restitution.
	float cor_average = (body_a->_coefficient_of_restitution + body_b->_coefficient_of_restitution) / 2.0f;

	
	if ((body_a->_flags & k_static) == 1)
	{
		// Handle body a being static
		float tmp1 = -1.0f * (cor_average + 1) * body_b->_mass * body_b->_velocity.dot(info->_normal);

		ga_vec3f p_hat = info->_normal.scale_result(tmp1);

		body_b->_velocity += (p_hat.scale_result(1.0f / body_b->_mass) * info->_normal);
	}
	else if ((body_b->_flags & k_static) == 1)
	{
		// Handle body b being static
		float tmp1 = -1.0f * (cor_average + 1) * body_a->_mass * body_a->_velocity.dot(info->_normal);

		ga_vec3f p_hat = info->_normal.scale_result(tmp1);

		body_a->_velocity += (p_hat.scale_result(1.0f / body_a->_mass) * info->_normal);
	}
	else
	{
		// Both bodies are not static
			
		float tmp1 = (cor_average + 1.0f) * (body_b->_velocity.dot(info->_normal) - body_a->_velocity.dot(info->_normal));
		float tmp2 = (1.0f / body_a->_mass) + (1.0f / body_b->_mass);

		ga_vec3f p_hat = info->_normal.scale_result(tmp1 / tmp2);
		
		ga_vec3f a_delta_v = p_hat.scale_result(1.0f / body_a->_mass);
		ga_vec3f b_delta_v = p_hat.scale_result(1.0f / body_b->_mass);

		body_a->_velocity = body_a->_velocity + a_delta_v;
		body_b->_velocity = body_b->_velocity - b_delta_v;

	}

}
