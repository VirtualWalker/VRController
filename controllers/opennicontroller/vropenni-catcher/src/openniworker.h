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
#include <QtDBus/QDBusConnection>
#include <QDebug>

#include "openniapplication.h"
#include "openniapplication_adaptor.h"
#include "openniapplicationdefines.h"

// Used to run the OpenNIApplication in a different thread
class OpenNIWorker : public QObject
{
        Q_OBJECT
    private:
        OpenNIApplication *_app = nullptr;
        bool _useAKinect;
        bool _firstSensor;
        int _frequency;
        USBDevicePath _camPath;
        USBDevicePath _motorPath;

    public:
        OpenNIWorker(int frequency, bool useAKinect, bool firstSensor, USBDevicePath camPath, USBDevicePath motorPath, QObject *parent = nullptr): QObject(parent)
        {
            _frequency = frequency;
            _useAKinect = useAKinect;
            _firstSensor = firstSensor;
            _camPath = camPath;
            _motorPath = motorPath;
        }

        ~OpenNIWorker()
        {
            // Wait for the frame loop stop
            while(!_app->isStopped())
            {}
            delete _app;
            _app = nullptr;
        }

        OpenNIApplication* app()
        {
            return _app;
        }

        void launch()
        {
            _app = new OpenNIApplication(_frequency, _useAKinect, _firstSensor, _camPath, _motorPath);

            if(_app->init() != XN_STATUS_OK)
                requestStop();
            if(_app->start() != XN_STATUS_OK)
                requestStop();
        }

        void moveToAngle(int angle)
        {
            if(_app != nullptr && _app->isInitialized())
                _app->moveToAngle(angle);
        }

        void requestStop()
        {
            if(_app != nullptr)
                _app->requestStop();
        }
};

// Only used for D-Bus functions
class OpenNIWorkerAdaptator : public QObject
{
        Q_OBJECT
    private:
        OpenNIWorker *_worker;
    signals:
        void needMotorAngleUpdate(int angle);

    public slots:
        void moveToAngle(int angle)
        {
            while(_worker->app() == nullptr || !_worker->app()->isInitialized())
            {}
            _worker->app()->moveToAngle(angle);
        }

    public:

        OpenNIWorkerAdaptator(OpenNIWorker* worker, QObject *parent = nullptr): QObject(parent)
        {
            _worker = worker;
        }
};

#endif // OPENNIWORKER

