/*
 * This file is part of VRController.
 * Copyright (c) 2015 Fabien Caylus <toutjuste13@gmail.com>
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONTROLLERCOMMON
#define CONTROLLERCOMMON

#define MIN_WALK_SPEED 0
#define MAX_WALK_SPEED 254

#define MIN_ORIENTATION 0
#define MAX_ORIENTATION 359

#define ORIENTATION_DECREASE_RATIO (255.0f/360.0f)
#define ORIENTATION_INCREASE_RATIO (360.0f/255.0f)

#define ORIENTATION_FORWARD 0
#define ORIENTATION_RIGHT 90
#define ORIENTATION_BACKWARD 180
#define ORIENTATION_LEFT 270

#define PI 3.14159265358979323846
#define DEG2RAD (PI/180.0)
#define RAD2DEG (180.0/PI)

#define PLUGINS_EXT_WITHOUT_DOT QStringLiteral("so")
#define PLUGINS_EXT QStringLiteral(".") + PLUGINS_EXT_WITHOUT_DOT
#define PLUGINS_PREFIX QStringLiteral("lib")
#ifdef QT_DEBUG
    #define PLUGINS_SUFFIX QStringLiteral("d")
#else
    #define PLUGINS_SUFFIX ""
#endif

#define UNUSED_VAR(x) (void)x

#endif // CONTROLLERCOMMON

