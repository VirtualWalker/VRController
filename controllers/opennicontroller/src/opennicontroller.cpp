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

#include "opennicontroller.h"
#include "controllercommon.h"

OpenNIController::OpenNIController(QObject *parent): ControllerInterface(parent)
{
}

OpenNIController::~OpenNIController()
{
    if(_widget != nullptr)
        _widget->deleteLater();
}

void OpenNIController::start()
{
    // Check if we are using a kinect sensor
    bool useAKinect = false;
    if(launchOptions().contains("useAKinect"))
        useAKinect = launchOptions()["useAKinect"][1].toBool();

    _widget = new OpenNIControllerWidget(dataFrequency(), useAKinect);
}

// Getters
int OpenNIController::orientation()
{
    return _widget->orientationValue();
}

int OpenNIController::walkSpeed()
{
    const int walkSpeed = _widget->walkSpeedValue();
    if (walkSpeed*2 > MAX_WALK_SPEED)
        return MAX_WALK_SPEED;
    return walkSpeed > MIN_COMPUTED_WALKSPEED ? walkSpeed*2 : 0;
}

int OpenNIController::specialCode()
{
    return _widget->specialCode();
}

QWidget *OpenNIController::widget()
{
    return _widget;
}

