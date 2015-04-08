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
        bool _useTwoSensors;

    public:
        OpenNIWorker(bool useAKinect = false, bool useTwoSensors = false, QObject *parent = nullptr): QObject(parent)
        {
            _useAKinect = useAKinect;
            _useTwoSensors = useTwoSensors;
        }

        ~OpenNIWorker()
        {
            // Wait for the frame loop stop
            while(!_app->stopped())
            {}
            delete _app;
            _app = nullptr;
        }

        int orientationValue()
        {
            if(_app != nullptr && _app->started())
                return _app->lastOrientation();
            return -1;
        }

        int walkSpeedValue()
        {
            if(_app != nullptr && _app->started())
                return _app->lastWalkSpeed();
            return -1;
        }

        OpenNIUtil::CameraInformations camInfo()
        {
            if(_app != nullptr && _app->started())
                return _app->lastCamInfo();
            return OpenNIUtil::createInvalidCamInfo();
        }

        OpenNIApplication* app()
        {
            return _app;
        }

    public slots:
        void launch()
        {
            _app = new OpenNIApplication(_useAKinect, _useTwoSensors);

            if(_app->init() != XN_STATUS_OK)
                requestStop();
            if(_app->start() != XN_STATUS_OK)
                requestStop();
        }

        void setMotorAngle(const int kinectID, const int angle)
        {
            if(_app != nullptr && _app->initialized())
                _app->moveToAngle(kinectID, angle);
        }

        void requestStop()
        {
            if(_app != nullptr)
                _app->requestStop();
        }

        void setAngleBetweenSensors(bool clockwise)
        {
            if(_app != nullptr)
                _app->setAngleBetweenSensors(clockwise);
        }
};

#endif // OPENNIWORKER

