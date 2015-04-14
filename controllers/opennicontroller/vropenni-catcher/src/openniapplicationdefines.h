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

#ifndef OPENNIAPPLICATIONDEFINES_H
#define OPENNIAPPLICATIONDEFINES_H

#define DBUS_ROOT_OBJECT "/OpenNIApplication"

#define DBUS_SERVICE_NAME_1 "fr.tjdev.VRController.OpenNIController.First"
#define DBUS_SERVICE_NAME_2 "fr.tjdev.VRController.OpenNIController.Second"

#define SHARED_MEM_DEPTH_1 "VRController-OpenNIController-Shared-Memmoty-One"
#define SHARED_MEM_DEPTH_2 "VRController-OpenNIController-Shared-Memmoty-Two"

#define SHARED_MEM_INFO_1 "VRController-OpenNIController-Shared-Memmoty-One-Info"
#define SHARED_MEM_INFO_2 "VRController-OpenNIController-Shared-Memmoty-Two-Info"

#define SHARED_MEM_DEPTH_SIZE (2*640*480)

#endif // OPENNIAPPLICATIONDEFINES_H

