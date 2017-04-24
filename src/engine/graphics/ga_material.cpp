/*
** RPI Game Architecture Engine
**
** Portions adapted from:
** Viper Engine - Copyright (C) 2016 Velan Studios - All Rights Reserved
**
** This file is distributed under the MIT License. See LICENSE.txt.
*/

#include "ga_material.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

void load_shader(const char* filename, std::string& contents)
{
	extern char g_root_path[256];
	std::string fullpath = g_root_path;
	fullpath += filename;

	std::ifstream file(fullpath);

	assert(file.is_open());

	contents.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

ga_unlit_texture_material::ga_unlit_texture_material(const char* texture_file) :
	_texture_file(texture_file)
{
}

ga_unlit_texture_material::~ga_unlit_texture_material()
{
}

bool ga_unlit_texture_material::init()
{
	std::string source_vs;
	load_shader("data/shaders/ga_unlit_texture_vert.glsl", source_vs);

	std::string source_fs;
	load_shader("data/shaders/ga_unlit_texture_frag.glsl", source_fs);

	_vs = new ga_shader(source_vs.c_str(), GL_VERTEX_SHADER);
	if (!_vs->compile())
	{
		std::cerr << "Failed to compile vertex shader:" << std::endl << _vs->get_compile_log() << std::endl;
	}

	_fs = new ga_shader(source_fs.c_str(), GL_FRAGMENT_SHADER);
	if (!_fs->compile())
	{
		std::cerr << "Failed to compile fragment shader:\n\t" << std::endl << _fs->get_compile_log() << std::endl;
	}

	_program = new ga_program();
	_program->attach(*_vs);
	_program->attach(*_fs);
	if (!_program->link())
	{
		std::cerr << "Failed to link shader program:\n\t" << std::endl << _program->get_link_log() << std::endl;
	}

	_texture = new ga_texture();
	if (!_texture->load_from_file(_texture_file.c_str()))
	{
		std::cerr << "Failed to load test.bmp" << std::endl;
	}

	return true;
}

void ga_unlit_texture_material::bind(const ga_mat4f& view_proj, const ga_mat4f& transform)
{
	ga_uniform mvp_uniform = _program->get_uniform("u_mvp");
	ga_uniform texture_uniform = _program->get_uniform("u_texture");

	_program->use();

	mvp_uniform.set(transform * view_proj);
	texture_uniform.set(*_texture, 0);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

ga_constant_color_material::ga_constant_color_material()
{
}

ga_constant_color_material::~ga_constant_color_material()
{
}

bool ga_constant_color_material::init()
{
	std::string source_vs;
	load_shader("data/shaders/ga_constant_color_vert.glsl", source_vs);

	std::string source_fs;
	load_shader("data/shaders/ga_constant_color_frag.glsl", source_fs);

	_vs = new ga_shader(source_vs.c_str(), GL_VERTEX_SHADER);
	if (!_vs->compile())
	{
		std::cerr << "Failed to compile vertex shader:" << std::endl << _vs->get_compile_log() << std::endl;
	}

	_fs = new ga_shader(source_fs.c_str(), GL_FRAGMENT_SHADER);
	if (!_fs->compile())
	{
		std::cerr << "Failed to compile fragment shader:\n\t" << std::endl << _fs->get_compile_log() << std::endl;
	}

	_program = new ga_program();
	_program->attach(*_vs);
	_program->attach(*_fs);
	if (!_program->link())
	{
		std::cerr << "Failed to link shader program:\n\t" << std::endl << _program->get_link_log() << std::endl;
	}

	return true;
}

void ga_constant_color_material::bind(const ga_mat4f& view_proj, const ga_mat4f& transform)
{
	ga_uniform mvp_uniform = _program->get_uniform("u_mvp");
	ga_uniform color_uniform = _program->get_uniform("u_color");

	_program->use();

	mvp_uniform.set(transform * view_proj);
	color_uniform.set(_color);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}

// ================================
// phong shader stuff
// ================================

ga_phong_color_material::ga_phong_color_material()
{
}

ga_phong_color_material::~ga_phong_color_material()
{
}

bool ga_phong_color_material::init()
{
	std::string source_vs;
	load_shader("data/shaders/ga_phong_color_vert.glsl", source_vs);

	std::string source_fs;
	load_shader("data/shaders/ga_phong_color_frag.glsl", source_fs);

	_vs = new ga_shader(source_vs.c_str(), GL_VERTEX_SHADER);
	if (!_vs->compile())
	{
		std::cerr << "Failed to compile vertex shader:" << std::endl << _vs->get_compile_log() << std::endl;
	}

	_fs = new ga_shader(source_fs.c_str(), GL_FRAGMENT_SHADER);
	if (!_fs->compile())
	{
		std::cerr << "Failed to compile fragment shader:\n\t" << std::endl << _fs->get_compile_log() << std::endl;
	}

	_program = new ga_program();
	_program->attach(*_vs);
	_program->attach(*_fs);
	if (!_program->link())
	{
		std::cerr << "Failed to link shader program:\n\t" << std::endl << _program->get_link_log() << std::endl;
	}

	return true;
}

void ga_phong_color_material::set_light_info(ga_vec3f light_pos, ga_vec3f ambient, ga_vec3f diffuse, ga_vec3f specular)
{
	light_info.Position = light_pos;
	light_info.La = ambient;
	light_info.Ld = diffuse;
	light_info.Ls = specular;
}

void ga_phong_color_material::set_material_info(ga_vec3f ambient_reflect, ga_vec3f diffuse_reflect, ga_vec3f specular_reflect, float shine)
{
	mat_info.Ka = ambient_reflect;
	mat_info.Kd = diffuse_reflect;
	mat_info.Ks = specular_reflect;
	mat_info.Shininess = shine;
}

void ga_phong_color_material::set_back_material_info(ga_vec3f ambient_reflect, ga_vec3f diffuse_reflect, ga_vec3f specular_reflect, float shine)
{
	back_mat_info.Ka = ambient_reflect;
	back_mat_info.Kd = diffuse_reflect;
	back_mat_info.Ks = specular_reflect;
	back_mat_info.Shininess = shine;
}

void ga_phong_color_material::bind(const ga_mat4f& view_proj, const ga_mat4f& transform)
{
	ga_uniform mvp_uniform = _program->get_uniform("u_mvp");

	ga_uniform light_pos_uniform = _program->get_uniform("LightPos");
	ga_uniform light_amib_uniform = _program->get_uniform("LightAmbient");
	ga_uniform light_diff_uniform = _program->get_uniform("LightDiffuse");
	ga_uniform light_spec_uniform = _program->get_uniform("LightSpecular");

	ga_uniform mat_amib_uniform = _program->get_uniform("MaterialAmbient");
	ga_uniform mat_diff_uniform = _program->get_uniform("MaterialDiffuse");
	ga_uniform mat_spec_uniform = _program->get_uniform("MaterialSpecular");
	//ga_uniform mat_shine_uniform = _program->get_uniform("MaterialShininess");

	ga_uniform back_mat_amib_uniform = _program->get_uniform("BackMaterialAmbient");
	ga_uniform back_mat_diff_uniform = _program->get_uniform("BackMaterialDiffuse");
	ga_uniform back_mat_spec_uniform = _program->get_uniform("BackMaterialSpecular");
	
	//ga_uniform lightInfo_uniform = _program->get_uniform("Light");
	//ga_uniform materialInfo_uniform = _program->get_uniform("Material");


	//ga_uniform mvp_uniform = _program->get_uniform("u_mvp");
	//ga_uniform color_uniform = _program->get_uniform("u_color");

	_program->use();

	mvp_uniform.set(transform * view_proj);
	light_pos_uniform.set(light_info.Position);
	light_amib_uniform.set(light_info.La);
	light_diff_uniform.set(light_info.Ld);
	light_spec_uniform.set(light_info.Ls);

	mat_amib_uniform.set(mat_info.Ka);
	mat_diff_uniform.set(mat_info.Kd);
	mat_spec_uniform.set(mat_info.Ks);

	back_mat_amib_uniform.set(back_mat_info.Ka);
	back_mat_diff_uniform.set(back_mat_info.Kd);
	back_mat_spec_uniform.set(back_mat_info.Ks);
	//mat_shine_uniform.set(mat_info.Shininess);
	//color_uniform.set(_color);
	//lightInfo_uniform.set(light_info);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}
