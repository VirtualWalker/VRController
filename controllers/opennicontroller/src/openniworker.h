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

#include "openniapplication.h"

// Used to manage OpenNI main loop
class OpenNIWorker : public QObject
{
        Q_OBJECT

    public:
        OpenNIWorker(int frequency, bool useAKinect = false, QObject *parent = nullptr);
        ~OpenNIWorker();

    public slots:
        void launch();

        void setMotorAngle(const int angle);
        void requestStop();

        int orientationValue();
        int walkSpeedValue();
        int specialCode();

        OpenNIUtil::CameraInformations camInfo();

    private:

        int _frequency;
        bool _useAKinect;
        int _specialCode = 0;

        OpenNIApplication *_app = nullptr;

};

#endif // OPENNIPROCESSWORKER

