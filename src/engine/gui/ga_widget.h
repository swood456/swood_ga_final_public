#pragma once

/*
** RPI Game Architecture Engine
**
** Portions adapted from:
** Viper Engine - Copyright (C) 2016 Velan Studios - All Rights Reserved
**
** This file is distributed under the MIT License. See LICENSE.txt.
*/

#include "math/ga_vec3f.h"

#include "ga_font.h"

#include "framework/ga_frame_params.h"

/*
** Base class for GUI widgets.
*/
class ga_widget
{
protected:
	void draw_box(float left_x, float right_x, float top_y, float bottom_y, ga_vec3f color, struct ga_frame_params* params);
};
