/*
** RPI Game Architecture Engine
**
** Portions adapted from:
** Viper Engine - Copyright (C) 2016 Velan Studios - All Rights Reserved
**
** This file is distributed under the MIT License. See LICENSE.txt.
*/

#include "ga_button.h"

#include "framework/ga_frame_params.h"


ga_button::ga_button(const char* text, float x, float y, ga_frame_params* params)
{

	// draw text
	extern ga_font* g_font;
	ga_vec3f text_color = { 1.0f, 1.0f, 1.0f };
	ga_vec2f top_left, bot_right;

	g_font->print(params, text, x, y, text_color, &top_left, &bot_right);


	_left_x = top_left.x - _edge_buffer;
	_right_x = bot_right.x + _edge_buffer;
	_upper_y = top_left.y - _edge_buffer;
	_lower_y = bot_right.y + _edge_buffer;
	

	// determine if the mouse is over the box to draw the color
	bool mouse_in_region = params->_mouse_x >= _left_x &&
		params->_mouse_y >= _upper_y &&
		params->_mouse_x <= _right_x &&
		params->_mouse_y <= _lower_y;

	if (mouse_in_region)
	{
		draw_box(_left_x, _right_x, _upper_y, _lower_y, { 0.0f, 0.0f, 1.0f }, params);

		if (params->_mouse_press_mask != 0) {
			// draw a box if the mouse is held down in the button
			ga_dynamic_drawcall bg_drawcall;

			bg_drawcall._positions.push_back({ top_left.x, top_left.y, 0.0f });
			bg_drawcall._positions.push_back({ bot_right.x, top_left.y, 0.0f });
			bg_drawcall._positions.push_back({ bot_right.x, bot_right.y, 0.0f });
			bg_drawcall._positions.push_back({ top_left.x, bot_right.y, 0.0f });

			bg_drawcall._indices.push_back(0);
			bg_drawcall._indices.push_back(1);
			bg_drawcall._indices.push_back(2);
			bg_drawcall._indices.push_back(2);
			bg_drawcall._indices.push_back(3);
			bg_drawcall._indices.push_back(0);
			
			bg_drawcall._color = { 0.0f, 0.0f, 0.8f };
			bg_drawcall._draw_mode = GL_TRIANGLES;
			bg_drawcall._transform.make_identity();
			bg_drawcall._material = nullptr;

			while (params->_gui_drawcall_lock.test_and_set(std::memory_order_acquire)) {}
			params->_gui_drawcalls.push_back(bg_drawcall);
			params->_gui_drawcall_lock.clear(std::memory_order_release);

			// re-draw text over colored box
			g_font->print(params, text, x, y, text_color, &top_left, &bot_right);
			
		}
	}
	else
	{
		draw_box(_left_x, _right_x, _upper_y, _lower_y, { 0.0f, 0.0f, 1.0f }, params);
	}

}

ga_button::~ga_button()
{
}

bool ga_button::get_clicked(const ga_frame_params* params) const
{
	bool click_in_region =
		params->_mouse_click_mask != 0 &&
		params->_mouse_x >= _left_x &&
		params->_mouse_y >= _upper_y &&
		params->_mouse_x <= _right_x &&
		params->_mouse_y <= _lower_y;

	return click_in_region;
}
