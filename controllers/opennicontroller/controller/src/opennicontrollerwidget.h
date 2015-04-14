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

#include "opencvwidget.h"
#include "opencvutil.h"
#include "openniprocessworker.h"

#include <QSpinBox>
#include <QThread>
#include <QRadioButton>
#include <QButtonGroup>

// Simple widget containing the image viewer
// This viewer also launch the OpenNI thread
class OpenNIControllerWidget: public QWidget
{
        Q_OBJECT
    public:
        explicit OpenNIControllerWidget(unsigned int frequency, bool useAKinect = false, bool useTwoSensors = false, QWidget *parent = nullptr);
        ~OpenNIControllerWidget();

        int orientationValue() const;
        int walkSpeedValue() const;
        int specialCode() const;

    protected:
        void timerEvent(QTimerEvent *event);

    private:

        OpenCVWidget *_viewer;

        OpenNIProcessWorker *_openniProcessWorker;
        QThread _openniThread;

        QSpinBox *_spinBox1;
        QSpinBox *_spinBox2;

        // This elements are only used if we use two sensors
        QButtonGroup *_angleButtonGroup;
        QRadioButton *_clockwiseButton;
        QRadioButton *_counterclockwiseButton;

        int _timerID = 0;

        bool _useTwoSensors;
};

#endif // OPENNICONTROLLERWIDGET_H
