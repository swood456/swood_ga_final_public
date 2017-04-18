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

private:
	void update_draw();

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

	// needed for drawing
	class ga_material* _material;
	uint32_t _vao;
	uint32_t _vbos[2];
	uint32_t _index_count;
};

// start of cloth.cpp that has to be here or it doesn't work
#include<iostream>

void ga_cloth_component::update_draw()
{
	std::cout << " doing an update for the draw, using subdata" << std::endl;
	
	//glGenVertexArrays(1, &_vao);

	int same = 0;
	same += 1;
	std::cout << "same: " << same << std::endl;
}
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

//		glm::vec3 ab = float(1 - x)*a + float(x)*b;
		//glm::vec3 dc = float(1 - x)*d + float(x)*c;

		for (int j = 0; j < ny; j++)
		{
			float y = j / double(ny - 1);
			ga_cloth_particle &p = get_particle(i, j);
			ga_vec3f abdc = ab.scale_result(1.0f - y) + dc.scale_result(y);
			p.set_original_position(abdc);
			p.set_position(abdc);
			p.set_velocity({ 0,0,0 });
			p.set_mass(mass);
			p.set_fixed(false);
		}
	}

	// drawing stuff, will need to be moved
	_material = new ga_constant_color_material();
	_material->init();
	_material->set_color({ 0.0f, 0.5f, 1.0f });

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

	// draw the cloth as a bunch of small triangles
	_index_count = indices.size();

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);

	glGenBuffers(2, _vbos);

	glBindBuffer(GL_ARRAY_BUFFER, _vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat) * 3, &verts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbos[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void ga_cloth_component::update(struct ga_frame_params* params)
{	
	ga_vec3f tl_pos = get_particle(0, 0).get_position();
	tl_pos -= {0.01f, 0.0f, 0.0f};
	get_particle(0, 0).set_position(tl_pos);
	//std::cout << "x: " << tl_pos.x << std::endl;
	update_draw();
	

	ga_static_drawcall draw;
	draw._name = "ga_cloth_component";
	draw._vao = _vao;
	draw._index_count = _index_count;
	draw._transform = get_entity()->get_transform();
	draw._draw_mode = GL_TRIANGLES;
	draw._material = _material;

	while (params->_static_drawcall_lock.test_and_set(std::memory_order_acquire)) {}
	params->_static_drawcalls.push_back(draw);
	params->_static_drawcall_lock.clear(std::memory_order_release);

	
}
ga_cloth_component::~ga_cloth_component()
{

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

	// First text case: A single box with no rotation falling to the floor.
	ga_entity test_1_box;
	test_1_box.translate({ -6.0f, 8.0f, 0.0f });

	ga_oobb test_1_oobb;
	test_1_oobb._half_vectors[0] = ga_vec3f::x_vector();
	test_1_oobb._half_vectors[1] = ga_vec3f::y_vector();
	test_1_oobb._half_vectors[2] = ga_vec3f::z_vector();

	ga_physics_component test_1_collider(&test_1_box, &test_1_oobb, 1.0f);

	world->add_rigid_body(test_1_collider.get_rigid_body());
	sim->add_entity(&test_1_box);

	// cloth stuff
	ga_entity cloth_ent;
	ga_cloth_component cloth_comp = ga_cloth_component(&cloth_ent, 1, 1, 1, 3, 3, {-4.0f, 8.0f, 0.0f}, { 2.0f, 8.0f, 0.0f }, { -4.0f, 2.0f, 0.0f }, { 2.0f, 2.0f, 0.0f }, 0.1f);
	//ga_cloth_component cloth_comp = ga_cloth_component(&cloth_ent, 1.0f);

	sim->add_entity(&cloth_ent);


	/*
	// Second test case: Two boxes with no rotation falling on top of each other.
	ga_entity test_2_box_1;
	test_2_box_1.translate({ 0.0f, 4.0f, 0.0f });
	ga_entity test_2_box_2;
	test_2_box_2.translate({ 0.0f, 8.0f, 0.0f });

	ga_oobb test_2_oobb_1;
	test_2_oobb_1._half_vectors[0] = ga_vec3f::x_vector();
	test_2_oobb_1._half_vectors[1] = ga_vec3f::y_vector();
	test_2_oobb_1._half_vectors[2] = ga_vec3f::z_vector();

	ga_oobb test_2_oobb_2;
	test_2_oobb_2._half_vectors[0] = ga_vec3f::x_vector();
	test_2_oobb_2._half_vectors[1] = ga_vec3f::y_vector();
	test_2_oobb_2._half_vectors[2] = ga_vec3f::z_vector();

	ga_physics_component test_2_collider_1(&test_2_box_1, &test_2_oobb_1, 3.0f);
	ga_physics_component test_2_collider_2(&test_2_box_2, &test_2_oobb_2, 1.0f);

	world->add_rigid_body(test_2_collider_1.get_rigid_body());
	world->add_rigid_body(test_2_collider_2.get_rigid_body());
	sim->add_entity(&test_2_box_1);
	sim->add_entity(&test_2_box_2);

	// Third test case: Two weightless boxes colliding off-center.
	ga_entity test_3_box_1;
	test_3_box_1.translate({ 5.6f, 4.0f, 0.2f });
	ga_entity test_3_box_2;
	test_3_box_2.translate({ 6.5f, 8.0f, 0.0f });

	ga_oobb test_3_oobb_1;
	test_3_oobb_1._half_vectors[0] = ga_vec3f::x_vector();
	test_3_oobb_1._half_vectors[1] = ga_vec3f::y_vector().scale_result(0.7f);
	test_3_oobb_1._half_vectors[2] = ga_vec3f::z_vector().scale_result(0.8f);

	ga_oobb test_3_oobb_2;
	test_3_oobb_2._half_vectors[0] = ga_vec3f::x_vector().scale_result(0.6f);
	test_3_oobb_2._half_vectors[1] = ga_vec3f::y_vector().scale_result(0.9f);
	test_3_oobb_2._half_vectors[2] = ga_vec3f::z_vector().scale_result(0.7f);

	ga_physics_component test_3_collider_1(&test_3_box_1, &test_3_oobb_1, 2.0f);
	ga_physics_component test_3_collider_2(&test_3_box_2, &test_3_oobb_2, 3.0f);
	test_3_collider_1.get_rigid_body()->make_weightless();
	test_3_collider_2.get_rigid_body()->make_weightless();

	test_3_collider_1.get_rigid_body()->add_linear_velocity({ 0.0f, 2.0f, 0.0f });
	test_3_collider_1.get_rigid_body()->add_angular_momentum({ 0.2f, 0.3f, 0.1f });
	test_3_collider_2.get_rigid_body()->add_linear_velocity({ 0.0f, -3.0f, 0.0f });

	world->add_rigid_body(test_3_collider_1.get_rigid_body());
	world->add_rigid_body(test_3_collider_2.get_rigid_body());
	sim->add_entity(&test_3_box_1);
	sim->add_entity(&test_3_box_2);
	*/
	// We need a floor for the boxes to fall onto.
	ga_entity floor;
	ga_plane floor_plane;
	floor_plane._point = { 0.0f, 0.0f, 0.0f };
	floor_plane._normal = { 0.0f, 1.0f, 0.0f };
	ga_physics_component floor_collider(&floor, &floor_plane, 0.0f);
	floor_collider.get_rigid_body()->make_static();
	world->add_rigid_body(floor_collider.get_rigid_body());
	sim->add_entity(&floor);

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

	world->remove_rigid_body(floor_collider.get_rigid_body());
	/*
	world->remove_rigid_body(test_3_collider_2.get_rigid_body());
	world->remove_rigid_body(test_3_collider_1.get_rigid_body());
	world->remove_rigid_body(test_2_collider_2.get_rigid_body());
	world->remove_rigid_body(test_2_collider_1.get_rigid_body());
	*/
	world->remove_rigid_body(test_1_collider.get_rigid_body());

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
