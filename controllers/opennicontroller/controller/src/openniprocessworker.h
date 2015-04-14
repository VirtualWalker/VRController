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

#ifndef OPENNIPROCESSWORKER
#define OPENNIPROCESSWORKER

#include <QObject>
#include <QProcess>
#include <QSharedMemory>
#include <QList>

#include "openniapplication_interface.h"
#include "../../vropenni-catcher/src/openniutil.h"

using namespace fr::tjdev::VRController::OpenNIController;

// Used to manage OpenNI sub-process in a different thread
class OpenNIProcessWorker : public QObject
{
        Q_OBJECT

    public:
        OpenNIProcessWorker(int frequency, bool useAKinect = false, bool useTwoSensors = false, QObject *parent = nullptr);
        ~OpenNIProcessWorker();

    public slots:
        void launch();

        void setMotorAngle(const int kinectID, const int angle);
        void requestStop();

        void setAngleBetweenSensors(bool clockwise);

        int orientationValue();
        int walkSpeedValue();
        int specialCode();

        OpenNIUtil::CameraInformations cameraInformations();
        OpenNIUtil::DepthMaps depthMaps();

    private:

        int _frequency;

        bool _useAKinect;
        bool _useTwoSensors;
        // Set to 1 or 2 inside the constructor
        int _sensorsCount;

        bool _stopped = false;

        int _specialCode = 0;

        // Tell if the angle between two kinects is clockwise or counterclockwise
        // 1 --> clockwise
        // -1 --> counterclockwise
        int _clockwise = 1;

        QList<QProcess *> _apps = QList<QProcess *>({nullptr, nullptr});

        // Shared memory segments
        QList<QSharedMemory *> _depthMemories = QList<QSharedMemory *>({nullptr, nullptr});
        QList<QSharedMemory *> _infoMemories = QList<QSharedMemory *>({nullptr, nullptr});

        // D-Bus Interfaces
        QList<OpenNIApplicationInterface *> _openniInterfaces = QList<OpenNIApplicationInterface *>({nullptr, nullptr});

        QList<bool> _started = QList<bool>({false, false});

        int _previousAverageRotation = -1;

    private slots:

        void newLogData(int id);
};

#endif // OPENNIPROCESSWORKER

