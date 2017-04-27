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

#include "physics/ga_cloth.h"
#include "graphics\ga_material.h"
#include "entity/ga_lua_component.h"

#include "gui/ga_font.h"
#include "gui/ga_label.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#if defined(GA_MINGW)
#include <unistd.h>
#endif

ga_font* g_font = nullptr;

static void set_root_path(const char* exepath);


int main(int argc, const char** argv)
{
	set_root_path(argv[0]);

	ga_job::startup(0xffff, 256, 256);

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


	////////// START CLOTHES ///////////////

	////////////////////////////////
	// BOX WITH A CAPE
	////////////////////////////////
	/*
	//lua box
	ga_entity lua;
	lua.translate({ 0.0f, 2.0f, 1.0f });
	ga_lua_component lua_move(&lua, "data/scripts/move.lua");
	ga_cube_component lua_model(&lua, "data/textures/rpi.png");
	sim->add_entity(&lua);

	// Make a new cloth entity that will follow the box
	ga_entity cape_ent;
	ga_cloth_component cloth_comp(&cape_ent, 2, 1, 1, 6, 12, { -1.0f, 1.0f, 1.1f }, { 1.0f, 1.0f, 1.1f }, { -1.0f, -2.0f, 1.02f }, { 1.0f, -2.0f, 1.02f }, 0.1f);
	ga_phong_color_material* cape_material = new ga_phong_color_material();
	cape_material->init();
	cape_material->set_light_info({ 0,0,0 }, { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 });
	cape_material->set_material_info({ 0.75f, 0.1f, 0.1f }, { 0.3f,0.3f,0.3f }, { 0,0,0 }, 0.2f);
	cape_material->set_back_material_info({ 0.3f, 0.1f, 0.1f }, { 0.3f,0.3f,0.3f }, { 0,0,0 }, 0.2f);
	cloth_comp.set_material(cape_material);

	// fix the cloth to his back
	cloth_comp.set_particle_fixed_ent(0, 0, &lua, { -1.0f, 1.0f, 1.1f });
	cloth_comp.set_particle_fixed_ent(5, 0, &lua, { 1.0f, 1.0f, 1.1f });
	sim->add_entity(&cape_ent);
	*/

	////////////////////////////////////
	// tablecloth - serial
	////////////////////////////////////
	
	ga_entity cloth_ent;
	// set up the cloth location and spring constants
	ga_cloth_component cloth_comp = ga_cloth_component(&cloth_ent, 2, 0.5, 0.01, 15, 15, { -5.0f,0.0f,-5.0f },
	{ 5.0f,0.0f,-5.0f }, { -5.0f,0.0f,5.0f }, { 5.0f,0.0f,5.0f }, 0.5f);

	// set up lighting and material color
	ga_phong_color_material* _material = new ga_phong_color_material();
	_material->init();
	_material->set_light_info({ -2.0f, 2, 2.0f }, { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 });
	_material->set_material_info({ 0.75f, 0.1f, 0.1f }, { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, 0.2f);
	_material->set_back_material_info({ 0.3f, 0.1f, 0.1f }, { 0.3f, 0.3f, 0.3f }, { 0, 0, 0 }, 0.2f);

	cloth_comp.set_material(_material);

	// set pats of the cloth to be fixed
	cloth_comp.set_particle_fixed(4, 4);
	cloth_comp.set_particle_fixed(4, 10);
	cloth_comp.set_particle_fixed(10, 4);
	cloth_comp.set_particle_fixed(10, 10);

	sim->add_entity(&cloth_ent);
	

	//////////////////////////////////////////
	// super parallelized table cloth
	//////////////////////////////////////////
	/*
	ga_entity cloth_ent;
	int n = 19;
	// set up the cloth location and spring constants
	ga_cloth_component cloth_comp = ga_cloth_component(&cloth_ent, 2, 0.5, 0.01, n, n, { -5.0f,0.0f,-5.0f },
	{ 5.0f,0.0f,-5.0f }, { -5.0f,0.0f,5.0f }, { 5.0f,0.0f,5.0f }, 0.5f);

	// set up lighting and material color
	ga_phong_color_material* _material = new ga_phong_color_material();
	_material->init();
	_material->set_light_info({ -2.0f, 2, 2.0f }, { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 });
	_material->set_material_info({ 0.75f, 0.1f, 0.1f }, { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, 0.2f);
	_material->set_back_material_info({ 0.3f, 0.1f, 0.1f }, { 0.3f, 0.3f, 0.3f }, { 0, 0, 0 }, 0.2f);

	cloth_comp.set_material(_material);

	// set pats of the cloth to be fixed
	int num_spots_from_edge = 5;
	cloth_comp.set_particle_fixed(num_spots_from_edge, num_spots_from_edge);
	cloth_comp.set_particle_fixed(num_spots_from_edge, n - num_spots_from_edge - 1);
	cloth_comp.set_particle_fixed(n - num_spots_from_edge - 1, num_spots_from_edge);
	cloth_comp.set_particle_fixed(n - num_spots_from_edge - 1, n - num_spots_from_edge - 1);

	cloth_comp.set_num_parallel_iters(3);

	sim->add_entity(&cloth_ent);
	*/
	///////////////////////////////////////////
	// simple cloth, mainly for initial testing
	///////////////////////////////////////////
	/*
	ga_entity cloth_ent;
	
	ga_cloth_component cloth_comp = ga_cloth_component(&cloth_ent, 1, 0.1f, 0.1f, 8, 8, { -4.0f, 8.0f, 0.0f }, { 2.0f, 8.0f, 0.0f }, { -4.0f, 2.0f, 0.0f }, { 2.0f, 2.0f, 0.0f }, 0.1f);
	
	ga_phong_color_material* _material = new ga_phong_color_material();
	_material->init();
	_material->set_light_info({ 0,0,0 }, { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 });
	_material->set_material_info({ 0.75f, 0.1f, 0.1f }, { 0.3f,0.3f,0.3f }, { 0,0,0 }, 0.2f);
	_material->set_back_material_info({ 0.3f, 0.1f, 0.1f }, { 0.3f,0.3f,0.3f }, { 0,0,0 }, 0.2f);

	cloth_comp.set_material(_material);

	cloth_comp.set_particle_fixed(0, 0,);

	sim->add_entity(&cloth_ent);
	*/

	///////////////////////////////////////////////
	// FLAG - rubbery and some self-intersection
	///////////////////////////////////////////////
	/*
	ga_entity flag_ent;
	// set up the cloth location and spring constants
	ga_cloth_component cloth_comp = ga_cloth_component(&flag_ent, 1, 0.3, 0.3, 15, 15, { -7.5f,5.0f,-5.0f },
	{ 10.0f,5.0f,-5.0f }, { -7.50f,-5.0f,-5.0f }, { 10.0f,-5.0f,-5.0f }, 0.5f);

	// set up lighting and material color
	ga_phong_color_material* flag_material = new ga_phong_color_material();
	flag_material->init();
	flag_material->set_light_info({ -2.0f, 2, 2.0f }, { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 });
	flag_material->set_material_info({ 0.75f, 0.1f, 0.1f }, { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, 0.2f);
	flag_material->set_back_material_info({ 0.3f, 0.1f, 0.1f }, { 0.3f, 0.3f, 0.3f }, { 0, 0, 0 }, 0.2f);

	cloth_comp.set_material(flag_material);

	// set pats of the cloth to be fixed
	cloth_comp.set_particle_fixed(0, 0, { -7.5f,5.0f,-4.0f });
	cloth_comp.set_particle_fixed(0, 14, { -7.50f,-5.0f,-4.0f });
	cloth_comp.set_particle_fixed(0, 4, { -7.50f,2.14f,-4.0f });
	cloth_comp.set_particle_fixed(0, 9, { -7.50f,-1.428f,-4.0f });

	sim->add_entity(&flag_ent);
	*/

	/////// END CLOTHES /////////////////


	// Create the default font:
	g_font = new ga_font("VeraMono.ttf", 16.0f, 512, 512);

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

		//gui
		float cloth_structural = cloth_comp.get_k_structural();
		ga_label(("structural: " + std::to_string(cloth_structural)).c_str(), 20.0f, 35.0f, &params);

		float cloth_sheer = cloth_comp.get_k_sheer();
		ga_label(("sheer: " + std::to_string(cloth_sheer)).c_str(), 20.0f, 50.0f, &params);

		float cloth_bend = cloth_comp.get_k_bend();
		ga_label(("bend: " + std::to_string(cloth_bend)).c_str(), 20.0f, 65.0f, &params);

		float fps = 1.0f / std::chrono::duration_cast<std::chrono::duration<float>>(params._delta_time).count();
		ga_label(("fps: " + std::to_string(fps)).c_str(), 20.0f, 20.0f, &params);

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