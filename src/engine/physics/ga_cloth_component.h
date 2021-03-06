#pragma once

#include "entity/ga_component.h"

#include <cstdint>
#include <cassert>

class ga_material;

/**
* Enum for type of integration to be used by cloth
**/
enum IntegrationType
{
	Euler,
	RK4_serial,
	RK4_parallel,
	Velocity_verlet
};

/**
* Cloth particle class
**/
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
	bool get_fixed_to_entity() const { return _is_fixed_to_entity; }
	ga_entity* get_other_entity() const { return _fixed_to_entity; }
	ga_vec3f get_offset() const { return _offset; }

	// modifiers
	void set_original_position(ga_vec3f pos) { _original_position = pos; }
	void set_position(ga_vec3f pos) { _position = pos; }
	void set_velocity(ga_vec3f vel) { _velocity = vel; }
	void set_acceleration(ga_vec3f acc) { _acceleration = acc; }
	void set_mass(float mass) { _mass = mass; }
	void set_fixed(bool fixed) { _is_fixed = fixed; }

	void set_fixed_to_other_entity(ga_entity* entity, ga_vec3f offset) {
		_is_fixed_to_entity = true;
		_fixed_to_entity = entity;
		_offset = offset;
	}

private:
	ga_vec3f _original_position;
	ga_vec3f _position;
	ga_vec3f _velocity;
	ga_vec3f _acceleration;
	float _mass;
	bool _is_fixed;

	bool _is_fixed_to_entity = false;
	ga_entity* _fixed_to_entity;
	ga_vec3f _offset;
};

/**
* Cloth component
**/
class ga_cloth_component : public ga_component
{
public:
	// Constructor
	ga_cloth_component(ga_entity* ent, float structural_k, float sheer_k, float bend_k, uint32_t nx, uint32_t ny,
		ga_vec3f top_left, ga_vec3f top_right, ga_vec3f bot_left, ga_vec3f bot_right, float fabric_weight);

	virtual ~ga_cloth_component();

	// Overriden ga_component update function
	virtual void update(struct ga_frame_params* params) override;

	/**
	*Public functions to allow cloth particles to be fixed to things
	**/
	// Sets cloth particle to be fixed at its current position
	void set_particle_fixed(int i, int j) { get_particle(i, j).set_fixed(true); }
	
	// Sets cloth particle to be fixed at a given position
	void set_particle_fixed(int i, int j, ga_vec3f fixed_pos) {
		get_particle(i, j).set_fixed(true);
		get_particle(i, j).set_position(fixed_pos);
	}
	
	// Sets a particle to be fixed relative to an entity with an offset
	void set_particle_fixed_ent(int i, int j, ga_entity* ent, ga_vec3f offset) {
		get_particle(i, j).set_fixed_to_other_entity(ent, offset);
	}

	// Public function to set up material
	void set_material(class ga_material* material) { _material = material; }

	// accessor functions to spring K values
	float get_k_structural() const { return _structural_k; }
	float get_k_sheer() const { return _sheer_k; }
	float get_k_bend() const { return _bend_k; }
	
	// Public functions to set up integration type and number of iterations
	void set_num_iterations(int n) { _num_iterations = n; }
	void set_integration_type(IntegrationType type) { _integration_type = type; }

private:

	// Enum for which type of integration
	IntegrationType _integration_type;
	
	// Various update functions
	void update_euler(struct ga_frame_params* params);
	void update_rk4(struct ga_frame_params* params);
	void update_rk4_row(struct ga_frame_params* params, uint32_t row);
	void update_velocity_verlet(struct ga_frame_params* params);
	void update_draw(struct ga_frame_params* params);

	// Helper functions to calculate various things in update functions
	ga_vec3f force_at_pos(int i, int j, ga_vec3f pos);
	ga_vec3f normal_for_point(int i, int j);
	ga_vec3f force_between_particles(int i, int j, int k, int l, float spring_k);
	ga_vec3f force_between_particles_at_pos(int i, int j, int k, int l, ga_vec3f pos, float spring_k);

	// private accessor for particles
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

	ga_vec3f _gravity;
	float _dampening;

	// needed for drawing
	class ga_material* _material;

	// num iterations for update
	int _num_iterations;
};
