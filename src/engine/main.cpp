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


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#if defined(GA_MINGW)
#include <unistd.h>
#endif

static void set_root_path(const char* exepath);
static void run_unit_tests();


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

	//lua box
	ga_entity lua;
	lua.translate({ 0.0f, 2.0f, 1.0f });
	ga_lua_component lua_move(&lua, "data/scripts/move.lua");
	ga_cube_component lua_model(&lua, "data/textures/rpi.png");
	sim->add_entity(&lua);

	// Make a new cloth entity that will follow the box
	ga_entity cape_ent;
	ga_cloth_component cape_cloth(&cape_ent, 2, 1, 1, 6, 12, { -1.0f, 1.0f, 1.1f }, { 1.0f, 1.0f, 1.1f }, { -1.0f, -2.0f, 1.02f }, { 1.0f, -2.0f, 1.02f }, 0.1f);
	ga_phong_color_material* cape_material = new ga_phong_color_material();
	cape_material->init();
	cape_material->set_light_info({ 0,0,0 }, { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 });
	cape_material->set_material_info({ 0.75f, 0.1f, 0.1f }, { 0.3f,0.3f,0.3f }, { 0,0,0 }, 0.2f);
	cape_material->set_back_material_info({ 0.3f, 0.1f, 0.1f }, { 0.3f,0.3f,0.3f }, { 0,0,0 }, 0.2f);
	cape_cloth.set_material(cape_material);

	// fix the cloth to his back
	cape_cloth.set_particle_fixed_ent(0, 0, &lua, { -1.0f, 1.0f, 1.1f });
	cape_cloth.set_particle_fixed_ent(5, 0, &lua, { 1.0f, 1.0f, 1.1f });
	//cape_cloth.set_particle_fixed(0, 0);
	//cape_cloth.set_particle_fixed(3, 0);
	sim->add_entity(&cape_ent);

	//////////////////////////////////
	// simple cloth
	//////////////////////////////////
	/*
	ga_entity cloth_ent;
	
	ga_cloth_component cloth_comp = ga_cloth_component(&cloth_ent, 1, 0.1f, 0.1f, 8, 8, { -4.0f, 8.0f, 0.0f }, { 2.0f, 8.0f, 0.0f }, { -4.0f, 2.0f, 0.0f }, { 2.0f, 2.0f, 0.0f }, 0.1f);
	
	ga_phong_color_material* _material = new ga_phong_color_material();
	_material->init();
	_material->set_light_info({ 0,0,0 }, { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 });
	_material->set_material_info({ 0.75f, 0.1f, 0.1f }, { 0.3f,0.3f,0.3f }, { 0,0,0 }, 0.2f);
	_material->set_back_material_info({ 0.3f, 0.1f, 0.1f }, { 0.3f,0.3f,0.3f }, { 0,0,0 }, 0.2f);

	cloth_comp.set_material(_material);

	cloth_comp.set_particle_fixed(0, 0, { -4.0f, 8.0f, -2.0f });

	sim->add_entity(&cloth_ent);
	*/

	////////////////////////////////////
	//med-res tablecloth
	////////////////////////////////////
	/*
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
	cloth_comp.set_particle_fixed(4, 4, { -2.5, 0, -2.5 });
	cloth_comp.set_particle_fixed(4, 10, { -2.5, 0, 2.5 });
	cloth_comp.set_particle_fixed(10, 4, { 2.5, 0, -2.5 });
	cloth_comp.set_particle_fixed(10, 10, { 2.5, 0, 2.5 });

	sim->add_entity(&cloth_ent);
	*/

	///////////////////////////////////////////////
	// FLAG - rubbery and some self-intersection
	///////////////////////////////////////////////
	/*
	ga_entity flag_ent;
	// set up the cloth location and spring constants
	ga_cloth_component flag_cloth = ga_cloth_component(&flag_ent, 1, 0.3, 0.3, 15, 15, { -7.5f,5.0f,-5.0f },
	{ 10.0f,5.0f,-5.0f }, { -7.50f,-5.0f,-5.0f }, { 10.0f,-5.0f,-5.0f }, 0.5f);

	// set up lighting and material color
	ga_phong_color_material* flag_material = new ga_phong_color_material();
	flag_material->init();
	flag_material->set_light_info({ -2.0f, 2, 2.0f }, { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 });
	flag_material->set_material_info({ 0.75f, 0.1f, 0.1f }, { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, 0.2f);
	flag_material->set_back_material_info({ 0.3f, 0.1f, 0.1f }, { 0.3f, 0.3f, 0.3f }, { 0, 0, 0 }, 0.2f);

	flag_cloth.set_material(flag_material);

	// set pats of the cloth to be fixed
	flag_cloth.set_particle_fixed(0, 0, { -7.5f,5.0f,-4.0f });
	flag_cloth.set_particle_fixed(0, 14, { -7.50f,-5.0f,-4.0f });
	flag_cloth.set_particle_fixed(0, 4, { -7.50f,2.14f,-4.0f });
	flag_cloth.set_particle_fixed(0, 9, { -7.50f,-1.428f,-4.0f });

	sim->add_entity(&flag_ent);
	*/

	///////////////////////
	// HIGH RES tablecloth
	///////////////////////
	/*
	ga_entity cloth_ent;
	ga_cloth_component cloth_comp = ga_cloth_component(&cloth_ent, 3, 0.5, 0.01, 21, 21, { -5.0f,0.0f,-5.0f }, { 5.0f,0.0f,-5.0f }, { -5.0f,0.0f,5.0f }, { 5.0f,0.0f,5.0f }, 0.7f);

	ga_phong_color_material* _material = new ga_phong_color_material();
	_material->init();
	_material->set_light_info({ -2.0f, 2, 2.0f }, { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 });
	_material->set_material_info({ 0.75f, 0.1f, 0.1f }, { 0.3f,0.3f,0.3f }, { 0,0,0 }, 0.2f);
	_material->set_back_material_info({ 0.3f, 0.1f, 0.1f }, { 0.3f,0.3f,0.3f }, { 0,0,0 }, 0.2f);

	cloth_comp.set_material(_material);

	cloth_comp.set_particle_fixed(5, 5, { -2.5, 0, -2.5 });
	cloth_comp.set_particle_fixed(5, 15, { -2.5, 0, 2.5 });
	cloth_comp.set_particle_fixed(15, 5, { 2.5, 0, -2.5 });
	cloth_comp.set_particle_fixed(15, 15, { 2.5, 0, 2.5 });

	sim->add_entity(&cloth_ent);
	*/
	///////////////////////////////////////////////////////////
	// Hanging Towel - does not look nice because of no self-collision
	///////////////////////////////////////////////////////////
	/*
	ga_entity towel_hanging_ent;
	ga_cloth_component towel_cloth_comp = ga_cloth_component(&towel_hanging_ent, 0.5, 0.2, 0.1, 11, 11, { -5.0f,5.0f,-5.0f }, { 5.0f,5.0f,-5.0f }, { -5.0f,5.0f,5.0f }, { 5.0f,5.0f,5.0f }, 0.5f);
	ga_phong_color_material* towel_material = new ga_phong_color_material();
	towel_material->init();
	towel_material->set_light_info({ -2.0f, 2, 2.0f }, { 1, 1, 1 }, { 1, 1, 1 }, { 1, 1, 1 });
	towel_material->set_material_info({ 0.75f, 0.1f, 0.1f }, { 0.3f,0.3f,0.3f }, { 0,0,0 }, 0.2f);
	towel_material->set_back_material_info({ 0.3f, 0.1f, 0.1f }, { 0.3f,0.3f,0.3f }, { 0,0,0 }, 0.2f);

	towel_cloth_comp.set_material(towel_material);
	towel_cloth_comp.set_particle_fixed(5, 0, { 0, 5, -5.0f });
	
	sim->add_entity(&towel_hanging_ent);
	*/
	/////////////////////////////////////
	// BIG tablecloth
	/////////////////////////////////////
	/*
	ga_entity cloth_ent;
	ga_cloth_component cloth_comp = ga_cloth_component(&cloth_ent, 2, 1, 1, 21, 21, { 0.0f,0.0f,0.0f }, { 10.0f,0.0f,0.0f }, { 0.0f,0.0f,10.0f }, { 10.0f,0.0f,10.0f }, 0.1f);
	cloth_comp.set_particle_fixed(5, 5, { 2.5, 0, 2.5 });
	cloth_comp.set_particle_fixed(5, 15, { 2.5, 0, 7.5 });
	cloth_comp.set_particle_fixed(15, 5, { 7.5, 0, 2.5 });
	cloth_comp.set_particle_fixed(15, 15, { 7.5, 0, 7.5 });

	sim->add_entity(&cloth_ent);
	*/
	

	////////////////////////////////////////////////////
	// silk curtain - does not look nice
	////////////////////////////////////////////////////
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

	//// END ADDING CLOTHES ////////////////////////////

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
