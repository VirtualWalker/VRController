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

OpenNIControllerWidget::OpenNIControllerWidget(unsigned int frequency, bool useAKinect, bool useTwoSensors, QWidget *parent): QWidget(parent)
{
    _useTwoSensors = useTwoSensors;
    _viewer = new OpenCVWidget(this);

    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(_viewer, 1);
    setLayout(mainLayout);

    QLabel *label = new QLabel(QString("<b>%1</b>").arg(tr("Motor orientation :")), this);
    label->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(label);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    mainLayout->addLayout(bottomLayout);

    _spinBox1 = new QSpinBox(this);
    _spinBox1->setRange(-30, 30);
    _spinBox1->setSuffix("°");
    _spinBox1->setValue(0);
    connect(_spinBox1, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int angle){
        _openniWorker->setMotorAngle(0, angle);
    });

    QFormLayout *layoutSensor1 = new QFormLayout();
    bottomLayout->addLayout(layoutSensor1);
    layoutSensor1->addRow(tr("Sensor %1 :").arg(1), _spinBox1);

    if(_useTwoSensors)
    {
        _spinBox2 = new QSpinBox(this);
        _spinBox2->setRange(-30, 30);
        _spinBox2->setSuffix("°");
        _spinBox2->setValue(0);
        connect(_spinBox2, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int angle){
            _openniWorker->setMotorAngle(1, angle);
        });

        QFormLayout *layoutSensor2 = new QFormLayout();
        bottomLayout->addLayout(layoutSensor2);
        layoutSensor2->addRow(tr("Sensor %1 :").arg(2), _spinBox2);

        _clockwiseButton = new QRadioButton(tr("Clockwise"), this);
        _counterclockwiseButton = new QRadioButton(tr("Counter-clockwise"), this);

        _angleButtonGroup = new QButtonGroup(this);
        _angleButtonGroup->addButton(_clockwiseButton, CLOCKWISE_BUTTON_ID);
        _angleButtonGroup->addButton(_counterclockwiseButton, COUNTERCLOCKWISE_BUTTON_ID);
        connect(_angleButtonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, [this](int id){
            _openniWorker->setAngleBetweenSensors(id == CLOCKWISE_BUTTON_ID);
        });

        QHBoxLayout *clockwiseLayout = new QHBoxLayout();
        clockwiseLayout->addWidget(_clockwiseButton);
        clockwiseLayout->addWidget(_counterclockwiseButton);

        QFormLayout *clockwiseFormLayout = new QFormLayout();
        clockwiseFormLayout->addRow(tr("Is the angle between sensor 2 and 1 clockwise or counterclockwises ?"), clockwiseLayout);
        mainLayout->addLayout(clockwiseFormLayout);
    }

    _openniWorker = new OpenNIWorker(useAKinect, _useTwoSensors);

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
