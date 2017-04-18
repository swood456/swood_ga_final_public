#pragma once

#include "entity/ga_component.h"

#include <cstdint>

class ga_cloth_component : public ga_component
{
public:
	/*
	ga_cloth_component(ga_entity* ent, float structural_k, float sheer_k, float bend_k, uint32_t nx, uint32_t ny,
		ga_vec3f top_left, ga_vec3f top_right, ga_vec3f bot_left, ga_vec3f bot_right, float fabric_weight);
	*/
	ga_cloth_component(ga_entity* ent, float k);
	virtual ~ga_cloth_component();

	virtual void update(struct ga_frame_params* params) override;

private:
	// representation of the mesh goes here
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


	// needed for drawing maybe
	class ga_material* _material;
	uint32_t _vao;
	uint32_t _vbos[4];
	uint32_t _index_count;
};
