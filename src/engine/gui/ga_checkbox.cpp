/*
** RPI Game Architecture Engine
**
** Portions adapted from:
** Viper Engine - Copyright (C) 2016 Velan Studios - All Rights Reserved
**
** This file is distributed under the MIT License. See LICENSE.txt.
*/

#include "ga_checkbox.h"

#include "framework/ga_frame_params.h"

ga_checkbox::ga_checkbox(bool state, const char* text, float x, float y, ga_frame_params* params)
{
	// draw checkbox
	_left_x = x;
	_right_x = x + _box_height;
	_upper_y = y;
	_lower_y = y + _box_height;


	// determine color based on if mouse is inside box
	bool mouse_in_region = params->_mouse_x >= _left_x &&
		params->_mouse_y >= _upper_y &&
		params->_mouse_x <= _right_x &&
		params->_mouse_y <= _lower_y;

	if (mouse_in_region)
	{
		draw_box(_left_x, _right_x, _upper_y, _lower_y, { 0.5f, 0.5f, 1.0f }, params);
	}
	else
	{
		draw_box(_left_x, _right_x, _upper_y, _lower_y, { 0.0f, 0.0f, 1.0f }, params);
	}

	// draw X if box is active
	if (state)
	{
		ga_dynamic_drawcall x_drawcall;

		x_drawcall._positions.push_back({ _left_x, _upper_y, 0.0f });
		x_drawcall._positions.push_back({ _right_x, _upper_y, 0.0f });
		x_drawcall._positions.push_back({ _right_x, _lower_y, 0.0f });
		x_drawcall._positions.push_back({ _left_x, _lower_y, 0.0f });

		x_drawcall._indices.push_back(0);
		x_drawcall._indices.push_back(2);
		x_drawcall._indices.push_back(1);
		x_drawcall._indices.push_back(3);

		if (mouse_in_region && params->_mouse_press_mask != 0)
		{
			x_drawcall._color = { 0.5f, 0.5f, 1.0f };
		}
		else
		{
			x_drawcall._color = { 0.0f, 0.0f, 1.0f };
			
		}
		
		x_drawcall._draw_mode = GL_LINES;
		x_drawcall._transform.make_identity();
		x_drawcall._material = nullptr;

		while (params->_gui_drawcall_lock.test_and_set(std::memory_order_acquire)) {}
		params->_gui_drawcalls.push_back(x_drawcall);
		params->_gui_drawcall_lock.clear(std::memory_order_release);
	}

	// draw the text
	extern ga_font* g_font;
	ga_vec3f text_color = { 1.0f, 1.0f, 1.0f };

	g_font->print(params, text, x + _box_buffer + _box_height, _lower_y, text_color, NULL, NULL);
	
	
}

ga_checkbox::~ga_checkbox()
{
}

bool ga_checkbox::get_clicked(const ga_frame_params* params) const
{
	bool click_in_region =
		params->_mouse_click_mask != 0 &&
		params->_mouse_x >= _left_x &&
		params->_mouse_y >= _upper_y &&
		params->_mouse_x <= _right_x &&
		params->_mouse_y <= _lower_y;

	return click_in_region;

}
