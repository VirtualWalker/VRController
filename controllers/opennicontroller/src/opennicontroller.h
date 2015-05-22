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

#ifndef OPENNICONTROLLER_H
#define OPENNICONTROLLER_H

#include "ControllerInterface"
#include "opennicontrollerwidget.h"
#include "controllercommon.h"

/**
 * This controller allow the use of the OpenNI SDK to send orientation and walk speed informations
 * to the connected device.
 */
class OpenNIController: public ControllerInterface
{
        Q_OBJECT
        Q_INTERFACES(ControllerInterface)
        Q_PLUGIN_METADATA(IID ControllerInterface_iid FILE "spec.json")

    private:
        OpenNIControllerWidget *_widget;

    public:
        explicit OpenNIController(QObject *parent = nullptr): ControllerInterface(parent)
        {
        }

        virtual ~OpenNIController()
        {
            if(_widget != nullptr)
                _widget->deleteLater();
        }

        void start()
        {
            _widget = new OpenNIControllerWidget(dataFrequency());
        }

        QWidget *widget()
        {
            return _widget;
        }

        int orientation()
        {
            return _widget->orientationValue();
        }

        int walkSpeed()
        {
            const int walkSpeed = _widget->walkSpeedValue();
            if (walkSpeed*2 > MAX_WALK_SPEED)
                return MAX_WALK_SPEED;
            return walkSpeed > MIN_COMPUTED_WALKSPEED ? walkSpeed*2 : 0;
        }

        int specialCode()
        {
            return _widget->specialCode();
        }
};

#endif // OPENNICONTROLLER_H
