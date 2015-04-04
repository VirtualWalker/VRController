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

#include "opennicontrollerwidget.h"
#include <QTimerEvent>
#include <QKeyEvent>
#include <QVBoxLayout>

OpenNIControllerWidget::OpenNIControllerWidget(unsigned int frequency, bool useAKinect, QWidget *parent): QWidget(parent)
{
    _viewer = new OpenCVWidget(this);

    setFocusPolicy(Qt::StrongFocus);
    grabKeyboard();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(_viewer);
    setLayout(layout);

    _openniWorker = new OpenNIWorker(useAKinect);

    connect(&_openniThread, &QThread::finished, _openniWorker, &QObject::deleteLater);
    connect(&_openniThread, &QThread::started, _openniWorker, &OpenNIWorker::launch);

    connect(_openniWorker, &OpenNIWorker::orientationChanged, this, &OpenNIControllerWidget::orientationChanged);
    connect(_openniWorker, &OpenNIWorker::walkSpeedChanged, this, &OpenNIControllerWidget::walkSpeedChanged);
    connect(_openniWorker, &OpenNIWorker::valueChanged, this, &OpenNIControllerWidget::valueChanged);

    _openniWorker->moveToThread(&_openniThread);
    _openniThread.start();

    // Start a timer
    _timerID = startTimer(1000/frequency, Qt::PreciseTimer);
}

OpenNIControllerWidget::~OpenNIControllerWidget()
{
    _openniWorker->requestStop();
    _openniThread.quit();
    _openniThread.wait();
}

// Getters
int OpenNIControllerWidget::orientationValue() const
{
    OpenNIApplication *app = _openniWorker->app();
    if(app != nullptr && app->started())
    {
        return app->lastOrientation();
    }
    return -1;
}

int OpenNIControllerWidget::walkSpeedValue() const
{
    OpenNIApplication *app = _openniWorker->app();
    if(app != nullptr && app->started())
    {
        return app->lastWalkSpeed();
    }
    return -1;
}

// Re-implemented protected method
void OpenNIControllerWidget::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == _timerID)
    {
        // Output the image
        OpenNIApplication *app = _openniWorker->app();
        if(app != nullptr && app->started())
        {
            cv::Mat image = OpenCVUtil::drawOpenNIData(app->lastCamInfo());
            _viewer->showImage(image);
        }
    }
}

void OpenNIControllerWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Left:
        case Qt::Key_Right:
            _openniWorker->needResetMotorAngle();
            break;
        case Qt::Key_Up:
            _openniWorker->needIncreaseMotorAngle();
            break;
        case Qt::Key_Down:
            _openniWorker->needDecreaseMotorAngle();
            break;
        default:
            break;
    }
    event->accept();
}
