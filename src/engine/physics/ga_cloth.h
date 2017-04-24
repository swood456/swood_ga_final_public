#pragma once

#include "entity/ga_component.h"

#include <cstdint>
#include <cassert>

class ga_material;

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
	ga_vec3f normal_for_point(int i, int j);

	ga_vec3f ga_cloth_component::force_between_particles(int i, int j, int k, int l, float spring_k);
	ga_vec3f ga_cloth_component::force_between_particles_at_pos(int i, int j, int k, int l, ga_vec3f pos, float spring_k);

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
