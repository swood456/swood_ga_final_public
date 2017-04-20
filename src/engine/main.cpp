/*
** RPI Game Architecture Engine
**
** Portions adapted from:
** Viper Engine - Copyright (C) 2016 Velan Studios - All Rights Reserved
**
** This file is distributed under the MIT License. See LICENSE.txt.
*/

#include "framework/ga_camera.h"
#include "framework/ga_compiler_defines.h"
#include "framework/ga_input.h"
#include "framework/ga_sim.h"
#include "framework/ga_output.h"
#include "jobs/ga_job.h"

#include "entity/ga_entity.h"

#include "graphics/ga_cube_component.h"
#include "graphics/ga_program.h"

#include "physics/ga_intersection.tests.h"
#include "physics/ga_physics_component.h"
#include "physics/ga_physics_world.h"
#include "physics/ga_rigid_body.h"
#include "physics/ga_shape.h"

//#include "physics/ga_cloth.h"
#include "graphics\ga_material.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#if defined(GA_MINGW)
#include <unistd.h>
#endif

static void set_root_path(const char* exepath);
static void run_unit_tests();


// start of cloth.h that does not work if it's not here
class ga_cloth_particle
{
public:
	// accessors
	const ga_vec3f& get_original_position() const { return _original_position; }
	const ga_vec3f& get_position() const { return _position; }
	const ga_vec3f& get_velocity() const { return _velocity; }
	const ga_vec3f& get_acceleration() const { return _acceleration; }
	float get_mass() const { return _mass; }
	bool get_fixed() const { return _is_fixed; }

	// modifiers
	void set_original_position(ga_vec3f pos) { _original_position = pos; }
	void set_position(ga_vec3f pos) { _position = pos; }
	void set_velocity(ga_vec3f vel) { _velocity = vel; }
	void set_acceleration(ga_vec3f acc) { _acceleration = acc; }
	void set_mass(float mass) { _mass = mass; }
	void set_fixed(bool fixed) { _is_fixed = fixed; }

private:
	ga_vec3f _original_position;
	ga_vec3f _position;
	ga_vec3f _velocity;
	ga_vec3f _acceleration;
	float _mass;
	bool _is_fixed;
};

class ga_cloth_component : public ga_component
{
public:
	
	ga_cloth_component(ga_entity* ent, float structural_k, float sheer_k, float bend_k, uint32_t nx, uint32_t ny,
		ga_vec3f top_left, ga_vec3f top_right, ga_vec3f bot_left, ga_vec3f bot_right, float fabric_weight);
	
	//ga_cloth_component(ga_entity* ent, float k);
	virtual ~ga_cloth_component();

	virtual void update(struct ga_frame_params* params) override;

	void set_particle_fixed(int i, int j, ga_vec3f fixed_pos) {
		get_particle(i, j).set_fixed(true);
		get_particle(i, j).set_position(fixed_pos);
	}

private:
	void update_euler(struct ga_frame_params* params);
	void update_rk4(struct ga_frame_params* params);

	void update_draw(struct ga_frame_params* params);
	ga_vec3f force_at_pos(int i, int j, ga_vec3f pos);
	
	ga_vec3f ga_cloth_component::force_between_particles(int i, int j, int k, int l, float spring_k);

	// private accessor
	ga_cloth_particle& get_particle(uint32_t i, uint32_t j)
	{
		assert(i >= 0 && i < _nx && j >= 0 && j < _ny);
		return _particles[i + j*_nx];
	}

	// cloth particle array
	ga_cloth_particle* _particles;

	// representation of the springs
	float _structural_k;
	float _sheer_k;
	float _bend_k;

	// number of particles in cloth along axes
	uint32_t _nx;
	uint32_t _ny;

	// locations of the cloth's corners
	ga_vec3f _top_left;
	ga_vec3f _top_right;
	ga_vec3f _bot_left;
	ga_vec3f _bot_right;

	float _fabric_weight;

	ga_vec3f _gravity;
	float _dampening;

	// needed for drawing
	class ga_material* _material;
};

// start of cloth.cpp that has to be here or it doesn't work
#include<iostream>
ga_cloth_component::ga_cloth_component(ga_entity* ent, float structural_k, float sheer_k, float bend_k, uint32_t nx, uint32_t ny,
	ga_vec3f top_left, ga_vec3f top_right, ga_vec3f bot_left, ga_vec3f bot_right, float fabric_weight) : ga_component(ent)
{
	_structural_k = structural_k;
	_sheer_k = sheer_k;
	_bend_k = bend_k;

	// number of particles in cloth along axes
	_nx = nx;
	_ny = ny;

	// locations of the cloth's corners
	_top_left = top_left;  // a
	_top_right = top_right;  // b
	_bot_left = bot_left;  // c
	_bot_right = bot_right;  // d

	_fabric_weight = fabric_weight;

	// set up the mesh of cloth particles
	_particles = new ga_cloth_particle[nx*ny];

	// this should ideally be changed to use cloth area somehow
	float mass = fabric_weight / (_nx * _ny);
	for (int i = 0; i < nx; i++)
	{
		float x = (float)i / (float)(nx - 1);
		ga_vec3f ab = top_left.scale_result(1.0f - x) + top_right.scale_result(x);
		ga_vec3f dc = bot_left.scale_result(1.0f - x) + bot_right.scale_result(x);

		for (int j = 0; j < ny; j++)
		{
			float y = j / double(ny - 1);
			ga_cloth_particle &p = get_particle(i, j);
			ga_vec3f abdc = ab.scale_result(1.0f - y) + dc.scale_result(y);
			p.set_original_position(abdc);
			p.set_position(abdc);
			p.set_velocity({ 0,0,0 });
			p.set_acceleration({ 0.0f, 0.0f, 0.0f });
			p.set_mass(mass);
			p.set_fixed(false);
		}
	}

	// drawing stuff, will need to be moved
	_material = new ga_constant_color_material();
	_material->init();
	_material->set_color({ 0.0f, 0.5f, 1.0f });

	_gravity = { 0.0f, 9.81f, 0.0f };

	_dampening = 0.01f;
}

void ga_cloth_component::update_draw(struct ga_frame_params* params)
{

	std::vector<ga_vec3f> verts;
	std::vector<GLushort> indices;

	for (int i = 1; i < _nx; i++)
	{
		for (int j = 1; j < _ny; j++)
		{
			uint32_t pos = verts.size();

			verts.push_back(get_particle(i - 1, j - 1).get_position());
			verts.push_back(get_particle(i, j - 1).get_position());
			verts.push_back(get_particle(i - 1, j).get_position());
			verts.push_back(get_particle(i, j).get_position());

			indices.push_back(pos);
			indices.push_back(pos + 1);
			indices.push_back(pos + 2);
			indices.push_back(pos + 2);
			indices.push_back(pos + 3);
			indices.push_back(pos + 1);

		}
	}

	ga_dynamic_drawcall draw;
	draw._name = "ga_cloth_dynamic";
	draw._color = { 0.0f, 0.5f, 1.0f };
	draw._material = _material;
	draw._positions = verts;
	draw._indices = indices;
	draw._transform = get_entity()->get_transform();
	draw._draw_mode = GL_TRIANGLES;
	
	while (params->_dynamic_drawcall_lock.test_and_set(std::memory_order_acquire)) {}
	params->_dynamic_drawcalls.push_back(draw);
	params->_dynamic_drawcall_lock.clear(std::memory_order_release);

}

ga_vec3f ga_cloth_component::force_at_pos(int i, int j, ga_vec3f pos)
{

	return{ 0,0,0 };
}

void ga_cloth_component::update_rk4(struct ga_frame_params* params)
{
	float dt = std::chrono::duration_cast<std::chrono::duration<float>>(params->_delta_time).count();

	for (int i = 0; i < _nx; i++)
	{
		for (int j = 0; j < _ny; j++)
		{
			ga_cloth_particle &p = get_particle(i, j);

			if (p.get_fixed())
			{
				continue;
			}

			//dt *= 0.1f;

			float p_mass = (float)p.get_mass();

			p.set_position(p.get_position() + p.get_velocity().scale_result(dt));

			p.set_velocity(p.get_velocity() + p.get_acceleration().scale_result(dt));


			//gravity
			ga_vec3f force_vec = _gravity.scale_result(p_mass * -1.0f);

			//structural springs
			force_vec += force_between_particles(i, j, i - 1, j, _structural_k);
			force_vec += force_between_particles(i, j, i + 1, j, _structural_k);
			force_vec += force_between_particles(i, j, i, j - 1, _structural_k);
			force_vec += force_between_particles(i, j, i, j + 1, _structural_k);

			//shear springs
			force_vec += force_between_particles(i, j, i - 1, j - 1, _sheer_k);
			force_vec += force_between_particles(i, j, i + 1, j - 1, _sheer_k);
			force_vec += force_between_particles(i, j, i - 1, j + 1, _sheer_k);
			force_vec += force_between_particles(i, j, i + 1, j + 1, _sheer_k);

			//bend springs
			force_vec += force_between_particles(i, j, i - 2, j, _bend_k);
			force_vec += force_between_particles(i, j, i + 2, j, _bend_k);
			force_vec += force_between_particles(i, j, i, j - 2, _bend_k);
			force_vec += force_between_particles(i, j, i, j + 2, _bend_k);

			//damping
			force_vec -= p.get_velocity().scale_result(_dampening);

			p.set_acceleration(force_vec.scale_result(1.0f / p.get_mass()));

		}
	}
}
void ga_cloth_component::update_euler(struct ga_frame_params* params)
{
	float dt = std::chrono::duration_cast<std::chrono::duration<float>>(params->_delta_time).count();

	dt *= 0.1f;

	for (int count = 0; count < 10; count++)
	{
		// update all the cloth position's positions
		for (int i = 0; i < _nx; i++)
		{
			for (int j = 0; j < _ny; j++)
			{
				ga_cloth_particle &p = get_particle(i, j);

				if (p.get_fixed())
				{
					continue;
				}

				//dt *= 0.1f;

				float p_mass = (float)p.get_mass();

				p.set_position(p.get_position() + p.get_velocity().scale_result(dt));

				p.set_velocity(p.get_velocity() + p.get_acceleration().scale_result(dt));


				//gravity
				ga_vec3f force_vec = _gravity.scale_result(p_mass * -1.0f);

				//structural springs
				force_vec += force_between_particles(i, j, i - 1, j, _structural_k);
				force_vec += force_between_particles(i, j, i + 1, j, _structural_k);
				force_vec += force_between_particles(i, j, i, j - 1, _structural_k);
				force_vec += force_between_particles(i, j, i, j + 1, _structural_k);

				//shear springs
				force_vec += force_between_particles(i, j, i - 1, j - 1, _sheer_k);
				force_vec += force_between_particles(i, j, i + 1, j - 1, _sheer_k);
				force_vec += force_between_particles(i, j, i - 1, j + 1, _sheer_k);
				force_vec += force_between_particles(i, j, i + 1, j + 1, _sheer_k);

				//bend springs
				force_vec += force_between_particles(i, j, i - 2, j, _bend_k);
				force_vec += force_between_particles(i, j, i + 2, j, _bend_k);
				force_vec += force_between_particles(i, j, i, j - 2, _bend_k);
				force_vec += force_between_particles(i, j, i, j + 2, _bend_k);

				//damping
				force_vec -= p.get_velocity().scale_result(_dampening);

				p.set_acceleration(force_vec.scale_result(1.0f / p.get_mass()));

			}
		}
	}
}

void ga_cloth_component::update(struct ga_frame_params* params)
{	
	update_euler(params);
	
	update_draw(params);
}
ga_cloth_component::~ga_cloth_component()
{

}

ga_vec3f ga_cloth_component::force_between_particles(int i, int j, int k, int l, float spring_k) {

	if (k < 0 || k >= _nx || l < 0 || l >= _ny) {
		return ga_vec3f{ 0.0f, 0.0f, 0.0f };
	}

	ga_cloth_particle &p1 = get_particle(i, j);
	ga_cloth_particle &p2 = get_particle(k, l);

	ga_vec3f distance = p2.get_position() - p1.get_position();

	float resting_length = (p2.get_original_position() - p1.get_original_position()).mag();

	ga_vec3f normalized_distance = distance.normal();

	return (distance - (normalized_distance.scale_result(resting_length))).scale_result(spring_k);
}

// end my mistake placement stuff

int main(int argc, const char** argv)
{
	set_root_path(argv[0]);

	ga_job::startup(0xffff, 256, 256);

	run_unit_tests();

	// Create objects for three phases of the frame: input, sim and output.
	ga_input* input = new ga_input();
	ga_sim* sim = new ga_sim();
	ga_physics_world* world = new ga_physics_world();
	ga_output* output = new ga_output(input->get_window());

	// Create camera.
	ga_camera* camera = new ga_camera({ 0.0f, 7.0f, 20.0f });
	ga_quatf rotation;
	rotation.make_axis_angle(ga_vec3f::y_vector(), ga_degrees_to_radians(180.0f));
	camera->rotate(rotation);
	rotation.make_axis_angle(ga_vec3f::x_vector(), ga_degrees_to_radians(15.0f));
	camera->rotate(rotation);

	// simple cloth
	
	ga_entity cloth_ent;
	ga_cloth_component cloth_comp = ga_cloth_component(&cloth_ent, 1, 1, 1, 5, 5, {-4.0f, 8.0f, 0.0f}, { 2.0f, 8.0f, 0.0f }, { -4.0f, 2.0f, 0.0f }, { 2.0f, 2.0f, 0.0f }, 0.1f);
	
	cloth_comp.set_particle_fixed(0, 0, { -4.0f, 8.0f, 0.0f });

	sim->add_entity(&cloth_ent);
	

	// silk curtain - does not look nice
	/*
	ga_entity silk_curtain_ent;
	ga_vec3f a = { 0, 8.0f, 0 };
	ga_vec3f b = { 8.0f, 8.0f, 0 };
	ga_vec3f c = { 8.0f, 2.0f, 0 };
	ga_vec3f d = { 0, 2.0f, 0 };
	ga_cloth_component silk_component = ga_cloth_component(&silk_curtain_ent, 1.0f, 0.1f, 0.01f, 22, 16, a, b, d, c, 0.07f);

	silk_component.set_particle_fixed(0, 0, { 1.6f, 8.0f, 0.1f });
	silk_component.set_particle_fixed(7, 0, { 4.8f, 8.0f, 0.1f });
	silk_component.set_particle_fixed(14, 0, { 4.8f, 8.0f, 0.1f });
	silk_component.set_particle_fixed(21, 0, { 6.4f, 8.0f, 0.1f });

	sim->add_entity(&silk_curtain_ent);
	*/

	// Main loop:
	while (true)
	{
		// We pass frame state through the 3 phases using a params object.
		ga_frame_params params;

		// Gather user input and current time.
		if (!input->update(&params))
		{
			break;
		}

		// Update the camera.
		camera->update(&params);

		// Run gameplay.
		sim->update(&params);

		// Step the physics world.
		world->step(&params);

		// Perform the late update.
		sim->late_update(&params);

		// Draw to screen.
		output->update(&params);
	}


	delete output;
	delete world;
	delete sim;
	delete input;
	delete camera;

	ga_job::shutdown();

	return 0;
}

char g_root_path[256];
static void set_root_path(const char* exepath)
{
#if defined(GA_MSVC)
	strcpy_s(g_root_path, sizeof(g_root_path), exepath);

	// Strip the executable file name off the end of the path:
	char* slash = strrchr(g_root_path, '\\');
	if (!slash)
	{
		slash = strrchr(g_root_path, '/');
	}
	if (slash)
	{
		slash[1] = '\0';
	}
#elif defined(GA_MINGW)
	char* cwd;
	char buf[PATH_MAX + 1];
	cwd = getcwd(buf, PATH_MAX + 1);
	strcpy_s(g_root_path, sizeof(g_root_path), cwd);

	g_root_path[strlen(cwd)] = '/';
	g_root_path[strlen(cwd) + 1] = '\0';
#endif
}

void run_unit_tests()
{
	// currently no plans for unit tests
	//ga_intersection_utility_unit_tests();
	//ga_intersection_unit_tests();
}
