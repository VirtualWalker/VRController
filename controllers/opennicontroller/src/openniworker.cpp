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

#include "openniworker.h"

#include <QCoreApplication>
#include <QProcess>
#include <QStringList>

OpenNIWorker::OpenNIWorker(int frequency, QObject *parent) : QObject(parent)
{
    _frequency = frequency;
}

OpenNIWorker::~OpenNIWorker()
{
    if(_app != nullptr)
    {
        // Wait for the frame loop stop
        while(!_app->isStopped())
        {}
        delete _app;
        _app = nullptr;
    }
}

void OpenNIWorker::launch()
{
    // List all available devices using the "lsusb" command
    QProcess lsusbProcess;
    lsusbProcess.start(QStringLiteral("lsusb"));
    lsusbProcess.waitForFinished(-1);
    QString lsusb = QString(lsusbProcess.readAllStandardOutput());

    /*
     * Sample output is:
     *
     * Bus 001 Device 011: ID 045e:02ae Microsoft Corp. Xbox NUI Camera
     * Bus 001 Device 007: ID 045e:02b0 Microsoft Corp. Xbox NUI Motor
     */

    QStringList sensorsLineList = lsusb.split("\n", QString::SkipEmptyParts);
    QList<USBDevicePath> motorsList;
    QList<USBDevicePath> camerasList;
    for(QString line : sensorsLineList)
    {
        if(line.contains(QStringLiteral("045e")) && line.contains(QStringLiteral("Microsoft Corp. Xbox NUI")))
        {
            // Cameras
            if(line.contains(QStringLiteral("ID 045e:02ae")))
            {
                camerasList.append(USBDevicePath({KINECT_VENDOR_ID, KINECT_CAMERA_PRODUCT_ID,
                                                  line.mid(4, 3).toInt(), line.mid(15, 3).toInt()}));
            }
            // Motors
            else if(line.contains(QStringLiteral("ID 045e:02b0")))
            {
                motorsList.append(USBDevicePath({KINECT_VENDOR_ID, KINECT_MOTOR_PRODUCT_ID,
                                                 line.mid(4, 3).toInt(), line.mid(15, 3).toInt()}));
            }
        }
    }

    // Debug infos
    qDebug() << qPrintable(tr("Sensors (camera):"));
    for(int i=0; i < camerasList.size(); ++i)
        qDebug() << qPrintable(QString("- %1").arg(camerasList[i].toString()));
    qDebug() << qPrintable(tr("Sensors (motor):"));
    for(int i=0; i < motorsList.size(); ++i)
        qDebug() << qPrintable(QString("- %1").arg(motorsList[i].toString()));

    // Check the number of sensors found
    if(motorsList.isEmpty() || camerasList.isEmpty())
    {
        qCritical() << qPrintable(tr("There is not enough connected sensors !"));
        return;
    }

    // Get the first sensor in lists
    _app = new OpenNIApplication(_frequency, camerasList[0], motorsList[0]);

    if(_app->init() != XN_STATUS_OK)
        requestStop();
    if(_app->start() != XN_STATUS_OK)
        requestStop();
}

void OpenNIWorker::setMotorAngle(const int angle)
{
    if(_app != nullptr && _app->isInitialized())
        _app->moveToAngle(angle);
}

void OpenNIWorker::requestStop()
{
    if(_app != nullptr)
        _app->requestStop();
}

int OpenNIWorker::orientationValue()
{
    return camInfo().user.rotation;
}

int OpenNIWorker::walkSpeedValue()
{
    return camInfo().user.walkSpeed;
}

int OpenNIWorker::specialCode()
{
    return _specialCode;
}

OpenNIUtil::CameraInformations OpenNIWorker::camInfo()
{
    if(_app == nullptr || !_app->isStarted())
        return OpenNIUtil::createInvalidCamInfo();

    OpenNIUtil::CameraInformations camInfo = _app->lastCamInfo();

    if(camInfo.user.numberOfFramesWithoutMove > 10)
        _specialCode = 2;
    else
        _specialCode = 0;

    return camInfo;
}
