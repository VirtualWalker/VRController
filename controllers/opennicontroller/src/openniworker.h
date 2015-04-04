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

#ifndef OPENNIWORKER
#define OPENNIWORKER

#include <QObject>
#include "openniapplication.h"

// Used to run the OpenNIApplication in a different thread
class OpenNIWorker : public QObject
{
        Q_OBJECT
    private:
        OpenNIApplication *_app = nullptr;
        bool _useAKinect;

    signals:
        void orientationChanged(int newOrientation);
        void walkSpeedChanged(int newWalkSpeed);
        void valueChanged();

    public slots:
        void launch()
        {
            _app = new OpenNIApplication(_useAKinect);

            connect(_app, &OpenNIApplication::orientationChanged, this, &OpenNIWorker::orientationChanged);
            connect(_app, &OpenNIApplication::walkSpeedChanged, this, &OpenNIWorker::walkSpeedChanged);
            connect(_app, &OpenNIApplication::camInfoChanged, this, &OpenNIWorker::valueChanged);

            _app->init();
            _app->start();
        }

        void needIncreaseMotorAngle()
        {
            if(_app != nullptr && _app->initialized())
                _app->increaseAngle();
        }

        void needDecreaseMotorAngle()
        {
            if(_app != nullptr && _app->initialized())
                _app->decreaseAngle();
        }

        void needResetMotorAngle()
        {
            if(_app != nullptr && _app->initialized())
                _app->resetAngle();
        }

    public:
        OpenNIWorker(bool useAKinect = false, QObject *parent = nullptr): QObject(parent)
        {
            _useAKinect = useAKinect;
        }

        ~OpenNIWorker()
        {
            // Wait for the frame loop stop
            while(!_app->stopped())
            {}
            delete _app;
            _app = nullptr;
        }

        void requestStop()
        {
            _app->requestStop();
        }

        OpenNIApplication* app()
        {
            return _app;
        }
};

#endif // OPENNIWORKER

