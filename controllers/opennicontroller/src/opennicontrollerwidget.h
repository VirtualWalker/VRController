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

#ifndef OPENNICONTROLLERWIDGET_H
#define OPENNICONTROLLERWIDGET_H

#include "ControllerInterface"
#include "openniworker.h"
#include "opencvwidget.h"
#include "opencvutil.h"

#include <QThread>

// Simple widget containing the image viewer
// This viewer also launch the OpenNI thread
class OpenNIControllerWidget: public QWidget
{
        Q_OBJECT
    public:
        explicit OpenNIControllerWidget(unsigned int frequency, bool useAKinect = false, QWidget *parent = nullptr);
        ~OpenNIControllerWidget();

        int orientationValue() const;
        int walkSpeedValue() const;

    signals:
        void orientationChanged(int newOrientation);
        void walkSpeedChanged(int newWalkSpeed);
        void valueChanged();

    protected:
        void timerEvent(QTimerEvent *event);

    private:

        OpenCVWidget *_viewer;
        // The worker class is used to launch the OpenNI loop in a different thread
        OpenNIWorker *_openniWorker;
        QThread _openniThread;

        int _timerID = 0;
};

#endif // OPENNICONTROLLERWIDGET_H
