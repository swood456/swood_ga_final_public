/*
** RPI Game Architecture Engine
**
** Portions adapted from:
** Viper Engine - Copyright (C) 2016 Velan Studios - All Rights Reserved
**
** This file is distributed under the MIT License. See LICENSE.txt.
*/

#include "ga_lua_component.h"

#include "entity/ga_entity.h"
#include "framework/ga_frame_params.h"

#include <lua.hpp>
#include <iostream>


int get_input_left(lua_State *L)
{
	// get the frame params from lua
	ga_frame_params* params;

	params = (ga_frame_params*) lua_topointer(L, 1);

	// push the left down boolean to the stack
	lua_pushboolean(L, params->_button_mask & k_button_j);

	return 1;
}

int get_input_right(lua_State *L)
{
	// get the frame params from lua
	ga_frame_params* params;
	params = (ga_frame_params*)lua_topointer(L, 1);

	// push the right down boolean to the stack
	lua_pushboolean(L, params->_button_mask & k_button_l);

	return 1;
}

int get_input_up(lua_State *L)
{
	// get the frame params from lua
	ga_frame_params* params;
	params = (ga_frame_params*)lua_topointer(L, 1);

	// push the right down boolean to the stack
	lua_pushboolean(L, params->_button_mask & k_button_i);

	return 1;
}

int get_input_down(lua_State *L)
{
	// get the frame params from lua
	ga_frame_params* params;
	params = (ga_frame_params*)lua_topointer(L, 1);

	// push the right down boolean to the stack
	lua_pushboolean(L, params->_button_mask & k_button_k);

	return 1;
}

int get_input_rot_left(lua_State *L)
{
	// get the frame params from lua
	ga_frame_params* params;
	params = (ga_frame_params*)lua_topointer(L, 1);

	// push the right down boolean to the stack
	lua_pushboolean(L, params->_button_mask & k_button_u);

	return 1;
}

int get_input_rot_right(lua_State *L)
{
	// get the frame params from lua
	ga_frame_params* params;
	params = (ga_frame_params*)lua_topointer(L, 1);

	// push the right down boolean to the stack
	lua_pushboolean(L, params->_button_mask & k_button_o);

	return 1;
}

int component_get_entity(lua_State *L)
{
	// get the component
	ga_component* component;
	component = (ga_component*)lua_touserdata(L, 1);

	// find the entity and push it to the stack
	ga_entity* entity = component->get_entity();
	lua_pushlightuserdata(L, entity);

	return 1;
}

int entity_translate(lua_State *L)
{
	// get the entity
	ga_entity* entity = (ga_entity*)lua_touserdata(L, 1);

	// get the translation values from lua
	ga_vec3f translation;

	translation.x = lua_tonumber(L, 2);
	translation.y = lua_tonumber(L, 3);
	translation.z = lua_tonumber(L, 4);

	// translate with those values
	entity->translate(translation);

	return 0;
}


int entity_rotate(lua_State *L)
{
	// get the entity
	ga_entity* entity = (ga_entity*)lua_touserdata(L, 1);

	// get the rotation values from lua
	float angle = lua_tonumber(L, 2);

	// rotate about y axis with this value
	ga_quatf axis_angle;
	axis_angle.make_axis_angle(ga_vec3f::y_vector(), angle);
	entity->rotate(axis_angle);

	return 0;
}

ga_lua_component::ga_lua_component(ga_entity* ent, const char* path) : ga_component(ent)
{
	// open script
	_lua = luaL_newstate();
	luaL_openlibs(_lua);

	// construct file path
	extern char g_root_path[256];
	std::string fullpath = g_root_path;
	fullpath += path;

	int status = luaL_loadfile(_lua, fullpath.c_str());

	if (status)
	{
		std::cerr << "Failed to load script " << path << ": " << lua_tostring(_lua, -1);
		lua_close(_lua);
	}

	// register the functions that lua will call
	lua_register(_lua, "frame_params_get_input_left", get_input_left);
	lua_register(_lua, "frame_params_get_input_right", get_input_right);
	lua_register(_lua, "frame_params_get_input_down", get_input_down);
	lua_register(_lua, "frame_params_get_input_up", get_input_up);
	lua_register(_lua, "frame_params_get_input_rot_left", get_input_rot_left);
	lua_register(_lua, "frame_params_get_input_rot_right", get_input_rot_right);
	lua_register(_lua, "component_get_entity", component_get_entity);
	lua_register(_lua, "entity_translate", entity_translate);
	lua_register(_lua, "entity_rotate", entity_rotate);


	// run the script that we pushed, so that we can call the
	//  functions in it
	status = lua_pcall(_lua, 0, LUA_MULTRET, 0);
	if (status)
	{
		std::cerr << "Failed to run script " << path << ": " << lua_tostring(_lua, -1);
		lua_close(_lua);
	}


}

ga_lua_component::~ga_lua_component()
{
	// close script
	lua_close(_lua);
}

void ga_lua_component::update(ga_frame_params* params)
{
	// put a call to update onto the stack
	lua_getglobal(_lua, "update");

	// put the component onto the stack
	lua_pushlightuserdata(_lua, this);

	// put the params onto the stack
	lua_pushlightuserdata(_lua, params);

	// run the stack
	int status = lua_pcall(_lua, 2, LUA_MULTRET, 0);
	if (status)
	{
		std::cerr << "Failed to run update function";
	}
}
