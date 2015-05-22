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

#ifndef FAKECONTROLLER_H
#define FAKECONTROLLER_H

#include "ControllerInterface"
#include "fakecontrollerwidget.h"

class FakeController: public ControllerInterface
{
        Q_OBJECT
        Q_INTERFACES(ControllerInterface)
        Q_PLUGIN_METADATA(IID ControllerInterface_iid FILE "spec.json")

    private:
        FakeControllerWidget *_widget;

    public:
        explicit FakeController(QObject *parent = nullptr) : ControllerInterface(parent) {}

        virtual ~FakeController()
        {
            if(_widget != nullptr)
                _widget->deleteLater();
        }

        void start()
        {
            _widget = new FakeControllerWidget();
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
            return _widget->walkSpeedValue();
        }

        // The special code is not used in this controller, since no calibration needed
        int specialCode()
        {
            return 0;
        }
};

#endif // FAKECONTROLLER_H
