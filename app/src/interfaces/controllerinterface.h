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
#include <QString>
#include <QMap>
#include <QPluginLoader>
#include <QVariant>

// Implements a basic interface used by all controllers.
class ControllerInterface: public QObject
{
        Q_OBJECT
        // This property is usually set by the program.
        // Used to know the frequency of data send with bluetooth
        Q_PROPERTY(unsigned int dataFrequency READ dataFrequency WRITE setDataFrequency)

    private:
        unsigned int _dataFrenquency = 1;

    public:

        explicit ControllerInterface(QObject *parent = nullptr): QObject(parent) {}
        // Used to start the controller
        // You can do some initialisations in this function
        virtual void start() = 0;
        virtual ~ControllerInterface() {}

        // Return the widget shown in the main window.
        virtual QWidget *widget() = 0;

        // Return the current orientation of the player.
        // Must be a number between 0 and 359.
        virtual int orientation() = 0;

        // Return the current walk speed of the player.
        // Must be a number between 0 and 254.
        virtual int walkSpeed() = 0;

        // Return the current special code
        // - 0: nothing
        // - 1: init the rotation
        // - 2: update the rotation with the specified one
        // - 3: start the game !
        virtual int specialCode() = 0;

        unsigned int dataFrequency() const
        {
            return _dataFrenquency;
        }

    public slots:
        void setDataFrequency(unsigned int frequency)
        {
            _dataFrenquency = frequency;
        }
};

#define ControllerInterface_iid "vrcontroller.controllerinterface"
Q_DECLARE_INTERFACE(ControllerInterface, ControllerInterface_iid)

#endif // CONTROLLERINTERFACE

