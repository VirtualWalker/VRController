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

#ifndef CONTROLLERINTERFACE
#define CONTROLLERINTERFACE

#include <QWidget>
#include <QObject>

// Implements a basic interface used by all controllers.
class ControllerInterface: public QObject
{
        Q_OBJECT

    public:

        explicit ControllerInterface(QObject *parent = nullptr): QObject(parent) {}
        virtual ~ControllerInterface() {}

        // Return the widget shown in the main window.
        virtual QWidget *widget() = 0;

        // Return the current orientation of the player.
        // Must be a number between 0 and 359.
        virtual int orientation() = 0;

        // Return the current walk speed of the player.
        // Must be a number between 0 and 254.
        virtual int walkSpeed() = 0;

    signals:

        void orientationChanged(int newOrientation);
        void walkSpeedChanged(int newWalkSpeed);
        // Emit when one of the above value are modified
        void somethingChanged();

};

#define ControllerInterface_iid "vrcontroller.controllerinterface"
Q_DECLARE_INTERFACE(ControllerInterface, ControllerInterface_iid)

#endif // CONTROLLERINTERFACE

