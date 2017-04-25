#include "ga_cloth.h"

#include "entity/ga_entity.h"

#include "graphics\ga_material.h"
#include <iostream>

ga_cloth_component::ga_cloth_component(ga_entity* ent, float structural_k, float sheer_k, float bend_k, uint32_t nx, uint32_t ny,
	ga_vec3f top_left, ga_vec3f top_right, ga_vec3f bot_left, ga_vec3f bot_right, float fabric_weight) : ga_component(ent)
{
	_structural_k = structural_k;
	_sheer_k = sheer_k;
	_bend_k = bend_k;

	// number of particles in cloth along axes
	_nx = nx;
	_ny = ny;

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

	_gravity = { 0.0f, 9.81f, 0.0f };

//	_dampening = 0.01f;
	_dampening = 0.008f;
	//_dampening = 0.005f;

	
}

void ga_cloth_component::set_material(ga_material* material) {
	_material = material;
}

ga_vec3f ga_cloth_component::normal_for_point(int i, int j)
{

	ga_vec3f norm_sum = { 0,0,0 };
	uint32_t num_norms = 0;

	if (i > 0 && j > 0)
	{
		norm_sum += ga_vec3f_cross(get_particle(i, j).get_position() - get_particle(i, j - 1).get_position(),
			get_particle(i, j).get_position() - get_particle(i - 1, j).get_position()).normal();
		num_norms++;
	}

	if (i > 0 && j < _ny - 1)
	{
		norm_sum += ga_vec3f_cross(get_particle(i, j).get_position() - get_particle(i - 1, j).get_position(),
			get_particle(i, j).get_position() - get_particle(i - 1, j + 1).get_position()).normal();
		num_norms++;

		norm_sum += ga_vec3f_cross(get_particle(i, j).get_position() - get_particle(i - 1, j + 1).get_position(),
			get_particle(i, j).get_position() - get_particle(i, j + 1).get_position()).normal();
		num_norms++;
	}

	if (i < _nx - 1 && j > 0)
	{
		norm_sum += ga_vec3f_cross(get_particle(i, j).get_position() - get_particle(i + 1, j - 1).get_position(),
			get_particle(i, j).get_position() - get_particle(i, j - 1).get_position()).normal();
		num_norms++;

		norm_sum += ga_vec3f_cross(get_particle(i, j).get_position() - get_particle(i + 1, j).get_position(),
			get_particle(i, j).get_position() - get_particle(i + 1, j - 1).get_position()).normal();
		num_norms++;
	}

	if (i < _nx - 1 && j < _ny - 1)
	{
		norm_sum += ga_vec3f_cross(get_particle(i, j).get_position() - get_particle(i, j + 1).get_position(),
			get_particle(i, j).get_position() - get_particle(i + 1, j).get_position()).normal();
		num_norms++;
	}


	if (num_norms > 0) {
		norm_sum.scale(1.0f / (float)num_norms);
	}

	return norm_sum.normal();
}

void ga_cloth_component::update_draw(struct ga_frame_params* params)
{
	std::vector<ga_vec3f> verts;
	std::vector<GLushort> indices;
	std::vector<ga_vec3f> norms;

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
			indices.push_back(pos + 2);
			indices.push_back(pos + 1);
			indices.push_back(pos + 1);
			indices.push_back(pos + 2);
			indices.push_back(pos + 3);

			norms.push_back(normal_for_point(i - 1, j - 1));
			norms.push_back(normal_for_point(i, j - 1));
			norms.push_back(normal_for_point(i - 1, j));
			norms.push_back(normal_for_point(i, j));
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
	draw._normals = norms;

	while (params->_dynamic_drawcall_lock.test_and_set(std::memory_order_acquire)) {}
	params->_dynamic_drawcalls.push_back(draw);
	params->_dynamic_drawcall_lock.clear(std::memory_order_release);

}

ga_vec3f ga_cloth_component::force_at_pos(int i, int j, ga_vec3f pos)
{
	ga_cloth_particle &p = get_particle(i, j);
	
	float p_mass = (float)p.get_mass();

	//gravity
	ga_vec3f force_vec = _gravity.scale_result(p_mass * -1.0f);

	//structural springs
	force_vec += force_between_particles_at_pos(i, j, i - 1, j, pos, _structural_k);
	force_vec += force_between_particles_at_pos(i, j, i + 1, j, pos, _structural_k);
	force_vec += force_between_particles_at_pos(i, j, i, j - 1, pos, _structural_k);
	force_vec += force_between_particles_at_pos(i, j, i, j + 1, pos, _structural_k);

	//shear springs
	force_vec += force_between_particles_at_pos(i, j, i - 1, j - 1, pos, _sheer_k);
	force_vec += force_between_particles_at_pos(i, j, i + 1, j - 1, pos, _sheer_k);
	force_vec += force_between_particles_at_pos(i, j, i - 1, j + 1, pos, _sheer_k);
	force_vec += force_between_particles_at_pos(i, j, i + 1, j + 1, pos, _sheer_k);

	//bend springs
	force_vec += force_between_particles_at_pos(i, j, i - 2, j, pos, _bend_k);
	force_vec += force_between_particles_at_pos(i, j, i + 2, j, pos, _bend_k);
	force_vec += force_between_particles_at_pos(i, j, i, j - 2, pos, _bend_k);
	force_vec += force_between_particles_at_pos(i, j, i, j + 2, pos, _bend_k);

	force_vec -= p.get_velocity().scale_result(_dampening);

	//ga_vec3f wind_f = { 0.3f, -0.1,0.0f };
	//force_vec += wind_f.scale_result(wind_f.dot(normal_for_point(i, j)));
	//force_vec += wind_f;

	return force_vec;
}
#include<iostream>
void ga_cloth_component::update_rk4(struct ga_frame_params* params)
{
	float dt = std::chrono::duration_cast<std::chrono::duration<float>>(params->_delta_time).count();

	int num_iterations_per_frame = 1;
	dt /= (float)num_iterations_per_frame;

	for (int k = 0; k < num_iterations_per_frame; k++) {

		//std::vector<std::vector<ga_vec3f>> new_pos;

		for (int i = 0; i < _nx; i++)
		{
			//std::vector<ga_vec3f> pos_row;
			for (int j = 0; j < _ny; j++)
			{
				ga_cloth_particle &p = get_particle(i, j);

				if (p.get_fixed())
				{
					//pos_row.push_back(p.get_position());
					continue;
				}
				
				float p_mass = p.get_mass();

				ga_vec3f p1 = p.get_position();
				ga_vec3f v1 = p.get_velocity();
				ga_vec3f a1 = force_at_pos(i, j, p1).scale_result(1.0f/ p_mass);

				ga_vec3f p2 = p1 + v1.scale_result(0.5f * dt);
				ga_vec3f v2 = v1 + a1.scale_result(0.5f * dt);
				ga_vec3f a2 = force_at_pos(i, j, p2).scale_result(1.0f / p_mass);

				ga_vec3f p3 = p1 + v2.scale_result(0.5f * dt);
				ga_vec3f v3 = v1 + a2.scale_result(0.5f * dt);
				ga_vec3f a3 = force_at_pos(i, j, p3).scale_result(1.0f / p_mass);

				ga_vec3f p4 = p1 + v3.scale_result(dt);
				ga_vec3f v4 = v1 + a3.scale_result(dt);
				ga_vec3f a4 = force_at_pos(i, j, p4).scale_result(1.0f / p_mass);

				p.set_position(p1 + (v1 + v2.scale_result(2) + v3.scale_result(2) + v4).scale_result(dt / 6.0f));
				//pos_row.push_back(p1 + (v1 + v2.scale_result(2) + v3.scale_result(2) + v4).scale_result(dt / 6.0f));
				
				p.set_velocity(v1 + (a1 + a2.scale_result(2) + a3.scale_result(2) + a4).scale_result(dt / 6.0f));

			}

			//new_pos.push_back(pos_row);
		}

		/*
		// may need provot corrections for this to work?
		for (int i = 0; i < _nx; i++) {
			for (int j = 0; j < _ny; j++) {
				get_particle(i, j).set_position(new_pos[i][j]);
			}
		}
		*/
		
	}
}
void ga_cloth_component::update_euler(struct ga_frame_params* params)
{
	float dt = std::chrono::duration_cast<std::chrono::duration<float>>(params->_delta_time).count();
	int num_euler = 20;
	dt /= num_euler;

	for (int count = 0; count < num_euler; count++)
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
	//update_euler(params);
	update_rk4(params);

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

ga_vec3f ga_cloth_component::force_between_particles_at_pos(int i, int j, int k, int l, ga_vec3f pos, float spring_k) {
	
	if (k < 0 || k >= _nx || l < 0 || l >= _ny) {
		return ga_vec3f{ 0.0f, 0.0f, 0.0f };
	}

	ga_cloth_particle &p1 = get_particle(i, j);
	ga_cloth_particle &p2 = get_particle(k, l);

	ga_vec3f distance = p2.get_position() - pos;

	float resting_length = (p2.get_original_position() - p1.get_original_position()).mag();

	ga_vec3f normalized_distance = distance.normal();

	return (distance - (normalized_distance.scale_result(resting_length))).scale_result(spring_k);
}