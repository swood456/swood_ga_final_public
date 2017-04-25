#pragma once

/*
** RPI Game Architecture Engine
**
** Portions adapted from:
** Viper Engine - Copyright (C) 2016 Velan Studios - All Rights Reserved
**
** This file is distributed under the MIT License. See LICENSE.txt.
*/

#include "ga_program.h"
#include "ga_texture.h"

#include "math/ga_mat4f.h"
#include "math/ga_vec3f.h"

#include <string>

/*
** Base class for all graphical materials.
** Includes the shaders and other state necessary to draw geometry.
*/
class ga_material
{
public:
	virtual bool init() = 0;

	virtual void bind(const ga_mat4f& view_proj, const ga_mat4f& transform) = 0;

	virtual void set_color(const ga_vec3f& color) {}

};

/*
** Simple unlit, single textured material.
*/
class ga_unlit_texture_material : public ga_material
{
public:
	ga_unlit_texture_material(const char* texture_file);
	~ga_unlit_texture_material();

	virtual bool init() override;

	virtual void bind(const ga_mat4f& view_proj, const ga_mat4f& transform) override;

private:
	std::string _texture_file;

	ga_shader* _vs;
	ga_shader* _fs;
	ga_program* _program;
	ga_texture* _texture;
};

/*
** Simple untextured material with a constant color.
*/
class ga_constant_color_material : public ga_material
{
public:
	ga_constant_color_material();
	~ga_constant_color_material();

	virtual bool init() override;

	virtual void bind(const ga_mat4f& view_proj, const ga_mat4f& transform) override;

	virtual void set_color(const ga_vec3f& color) override { _color = color; }

private:
	ga_shader* _vs;
	ga_shader* _fs;
	ga_program* _program;
	ga_vec3f _color;
};

class ga_phong_color_material : public ga_material
{
public:
	ga_phong_color_material();
	~ga_phong_color_material();

	virtual bool init() override;

	virtual void bind(const ga_mat4f& view_proj, const ga_mat4f& transform) override;

	virtual void set_color(const ga_vec3f& color) override { _color = color; }

	void set_light_info(ga_vec3f light_pos, ga_vec3f ambient, ga_vec3f diffuse, ga_vec3f specular);

	void set_material_info(ga_vec3f ambient_reflect, ga_vec3f diffuse_reflect, ga_vec3f specular_reflect, float shine);

	void set_back_material_info(ga_vec3f ambient_reflect, ga_vec3f diffuse_reflect, ga_vec3f specular_reflect, float shine);

	void set_light_pos(ga_vec3f lp) { light_info.Position = lp; }

private:
	struct LightInfo
	{
		ga_vec3f Position;	//Light Position in eye-coords
		ga_vec3f La;		//Ambient light intensity
		ga_vec3f Ld;		//Diffuse light intensity
		ga_vec3f Ls;		//Specular light intensity
	} light_info;

	struct MaterialInfo
	{
		ga_vec3f Ka;			//Ambient reflectivity
		ga_vec3f Kd;			//Diffuse reflectivity
		ga_vec3f Ks;			//Specular reflectivity
		float Shininess;		//Specular shininess factor
	} mat_info;

	struct MaterialInfo back_mat_info;

	ga_shader* _vs;
	ga_shader* _fs;
	ga_program* _program;
	ga_vec3f _color;
};