#include "ga_cloth.h"

#include "entity/ga_entity.h"

#include <iostream>
/*
ga_cloth_component::ga_cloth_componentga_entity* ent, float structural_k, float sheer_k, float bend_k, uint32_t nx, uint32_t ny,
	ga_vec3f top_left, ga_vec3f top_right, ga_vec3f bot_left, ga_vec3f bot_right, float fabric_weight) : ga_component(ent)
{
	_structural_k = structural_k;
	_sheer_k = sheer_k;
	_bend_k = bend_k;

	// number of particles in cloth along axes
	_nx = nx;
	_ny = ny;

	// locations of the cloth's corners
	_top_left = top_left;
	_top_right = top_right;
	_bot_left = bot_left;
	_bot_right = bot_right;

	_fabric_weight = fabric_weight;
}
*/
ga_cloth_component::ga_cloth_component(ga_entity* ent, float k)
	: ga_component(ent)
{
	_structural_k = k;
}

ga_cloth_component::~ga_cloth_component()
{

}

void ga_cloth_component::update(struct ga_frame_params* params)
{
	std::cout << " cloth update" << std::endl;
}
