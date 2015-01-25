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

#include "fakecontroller.h"

FakeController::FakeController(QObject *parent): ControllerInterface(parent)
{
    _widget = new FakeControllerWidget();
    connect(_widget, &FakeControllerWidget::orientationChanged, this, &ControllerInterface::orientationChanged);
    connect(_widget, &FakeControllerWidget::walkSpeedChanged, this, &ControllerInterface::walkSpeedChanged);
    connect(_widget, &FakeControllerWidget::valueChanged, this, &ControllerInterface::somethingChanged);
}

FakeController::~FakeController()
{
    _widget->deleteLater();
}

// Getters
int FakeController::orientation()
{
    return _widget->orientationValue();
}

int FakeController::walkSpeed()
{
    return _widget->walkSpeedValue();
}

QWidget *FakeController::widget()
{
    return _widget;
}

