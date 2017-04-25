/*
** RPI Game Architecture Engine
**
** Portions adapted from:
** Viper Engine - Copyright (C) 2016 Velan Studios - All Rights Reserved
**
** This file is distributed under the MIT License. See LICENSE.txt.
*/

#include "ga_label.h"

#include "framework/ga_frame_params.h"

ga_label::ga_label(const char* text, float x, float y, ga_frame_params* params)
{
	extern ga_font* g_font;
	ga_vec3f text_color = { 1.0f, 1.0f, 1.0f };

	g_font->print(params, text, x, y, text_color, NULL, NULL);
}

ga_label::~ga_label()
{
}
