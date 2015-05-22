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
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>

#define CLOCKWISE_BUTTON_ID 12
#define COUNTERCLOCKWISE_BUTTON_ID 20

OpenNIControllerWidget::OpenNIControllerWidget(unsigned int frequency, QWidget *parent): QWidget(parent)
{
    _viewer = new OpenCVWidget(this);

    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_viewer, 1);
    setLayout(mainLayout);

    _spinBox = new QSpinBox(this);
    _spinBox->setRange(-30, 30);
    _spinBox->setSuffix(QStringLiteral("°"));
    _spinBox->setValue(0);
    connect(_spinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int angle){
        _openniWorker->setMotorAngle(angle);
    });

    QFormLayout *layoutSensor = new QFormLayout();
    mainLayout->addLayout(layoutSensor);
    layoutSensor->addRow(QString("<b>%1</b>").arg(tr("Motor orientation :")), _spinBox);

    _openniWorker = new OpenNIWorker(frequency);

    connect(&_openniThread, &QThread::finished, _openniWorker, &QObject::deleteLater);
    connect(&_openniThread, &QThread::started, _openniWorker, &OpenNIWorker::launch);

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
    return _openniWorker->orientationValue();
}

int OpenNIControllerWidget::walkSpeedValue() const
{
    return _openniWorker->walkSpeedValue();
}

int OpenNIControllerWidget::specialCode() const
{
    return _openniWorker->specialCode();
}

// Re-implemented protected method
void OpenNIControllerWidget::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == _timerID)
    {
        // Output the image
        OpenNIUtil::CameraInformations camInfo = _openniWorker->camInfo();
        if(!camInfo.invalid)
        {
            cv::Mat image = OpenCVUtil::drawOpenNIData(camInfo);
            _viewer->showImage(image);
        }
    }
}
