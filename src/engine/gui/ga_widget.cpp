/*
** RPI Game Architecture Engine
**
** Portions adapted from:
** Viper Engine - Copyright (C) 2016 Velan Studios - All Rights Reserved
**
** This file is distributed under the MIT License. See LICENSE.txt.
*/

#include "ga_widget.h"

void ga_widget::draw_box(float left_x, float right_x, float top_y, float bottom_y, ga_vec3f color, struct ga_frame_params* params)
{
	ga_dynamic_drawcall drawcall;

	drawcall._positions.push_back({ left_x, top_y, 0.0f });
	drawcall._positions.push_back({ right_x, top_y, 0.0f });
	drawcall._positions.push_back({ right_x, bottom_y, 0.0f });
	drawcall._positions.push_back({ left_x, bottom_y, 0.0f });

	drawcall._indices.push_back(0);
	drawcall._indices.push_back(1);
	drawcall._indices.push_back(1);
	drawcall._indices.push_back(2);
	drawcall._indices.push_back(2);
	drawcall._indices.push_back(3);
	drawcall._indices.push_back(3);
	drawcall._indices.push_back(0);

	drawcall._color = color;
	drawcall._draw_mode = GL_LINES;
	drawcall._transform.make_identity();
	drawcall._material = nullptr;

	while (params->_gui_drawcall_lock.test_and_set(std::memory_order_acquire)) {}
	params->_gui_drawcalls.push_back(drawcall);
	params->_gui_drawcall_lock.clear(std::memory_order_release);
}