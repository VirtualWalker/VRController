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

OpenNIController::OpenNIController(QObject *parent): ControllerInterface(parent)
{
}

OpenNIController::~OpenNIController()
{
    _widget->deleteLater();
}

void OpenNIController::start()
{
    // Check if we are using a kinect sensor
    bool useAKinect = false;
    if(launchOptions().contains("useAKinect"))
        useAKinect = launchOptions()["useAKinect"].second;

    _widget = new OpenNIControllerWidget(dataFrequency(), useAKinect);
    connect(_widget, &OpenNIControllerWidget::orientationChanged, this, &ControllerInterface::orientationChanged);
    connect(_widget, &OpenNIControllerWidget::walkSpeedChanged, this, &ControllerInterface::walkSpeedChanged);
    connect(_widget, &OpenNIControllerWidget::valueChanged, this, &ControllerInterface::somethingChanged);
}

// Getters
int OpenNIController::orientation()
{
    return _widget->orientationValue();
}

int OpenNIController::walkSpeed()
{
    return _widget->walkSpeedValue();
}

QWidget *OpenNIController::widget()
{
    return _widget;
}
