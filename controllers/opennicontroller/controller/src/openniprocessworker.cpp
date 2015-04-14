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

#include "openniprocessworker.h"
#include "../../vropenni-catcher/src/usbcontroller.h"
#include "../../vropenni-catcher/src/openniapplicationdefines.h"

#include <QCoreApplication>
#include <QStringList>
#include <QList>
#include <QDBusConnection>
#include <QDataStream>

#include <cstring>

using namespace fr::tjdev::VRController::OpenNIController;

OpenNIProcessWorker::OpenNIProcessWorker(int frequency, bool useAKinect, bool useTwoSensors, QObject *parent) : QObject(parent)
{
    _frequency = frequency;
    _useAKinect = useAKinect;
    _useTwoSensors = useTwoSensors;
    _sensorsCount = _useTwoSensors ? 2 : 1;

    if(_useAKinect)
            qDebug() << qPrintable(tr("The application will assume that a kinect sensor is used !"));

    if(!_useAKinect && _useTwoSensors)
    {
        qCritical() << qPrintable(tr("You must use Kinect sensors if you want to use two sensors !"));
        return;
    }

    if(_useTwoSensors)
    {
        qDebug() << qPrintable(tr("The application will use two sensors !"));
        qDebug() << qPrintable(tr("The angle between the sensor 2 and the sensor 1 must be of 90°."));
        qDebug() << qPrintable(tr("You can tell if this angle is clockwise or counterclockwise inside the GUI."));
    }
}

OpenNIProcessWorker::~OpenNIProcessWorker()
{
    for(int i=0; i<_sensorsCount; ++i)
    {
        if(_apps[i] != nullptr)
        {
            _apps[i]->terminate();
            _apps[i]->waitForFinished();
            _apps[i]->deleteLater();
        }

        if(_depthMemories[i] != nullptr)
        {
            _depthMemories[i]->detach();
            _depthMemories[i]->deleteLater();
        }

        if(_infoMemories[i] != nullptr)
        {
            _infoMemories[i]->detach();
            _infoMemories[i]->deleteLater();
        }
    }
}

void OpenNIProcessWorker::launch()
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
    if(_sensorsCount > motorsList.size() || _sensorsCount > camerasList.size() || camerasList.size() != motorsList.size())
    {
        qCritical() << qPrintable(tr("There is not enough connected sensors !"));
        return;
    }

    for(int i=0 ; i<_sensorsCount ; ++i)
    {
        // Create Depth memory
        _depthMemories[i] = new QSharedMemory((i == 0) ? SHARED_MEM_DEPTH_1 : SHARED_MEM_DEPTH_2, this);
        if(!_depthMemories[i]->create(SHARED_MEM_DEPTH_SIZE))
        {
            if(_depthMemories[i]->error() == QSharedMemory::AlreadyExists)
                _depthMemories[i]->attach();
            else
            {
                qCritical() << qPrintable(tr("Unable to create depth shared memory for sensor %1. Error: %2").arg(i).arg(_depthMemories[i]->errorString()));
                return;
            }
        }

        // Create info memory
        _infoMemories[i] = new QSharedMemory((i == 0) ? SHARED_MEM_INFO_1 : SHARED_MEM_INFO_2, this);
        if(!_infoMemories[i]->create(sizeof(OpenNIUtil::CameraInformations)))
        {
            if(_infoMemories[i]->error() == QSharedMemory::AlreadyExists)
                _infoMemories[i]->attach();
            else
            {
                qCritical() << qPrintable(tr("Unable to create information shared memory for sensor %1. Error: %2").arg(i).arg(_infoMemories[i]->errorString()));
                return;
            }
        }

        // Launch separate process for each sensor
        _apps[i] = new QProcess();
        _apps[i]->setReadChannel(QProcess::StandardError);
        connect(_apps[i], &QProcess::readyReadStandardError, this, [this, i](){
            newLogData(i);
        });
        // Arguments are:
        //   useAKinect firstSensor camVendor camProduct camBus camDevice motorVendor motorProduct motorBus motorDevice
        _apps[i]->start(QCoreApplication::applicationDirPath() + "/vropenni-catcher",
                        QStringList() << QString::number(_frequency)
                                      << (_useAKinect ? "true" : "false")
                                      << ((i == 0) ? "true" : "false")
                                      << QString::number(camerasList[i].vendor)
                                      << QString::number(camerasList[i].product)
                                      << QString::number(camerasList[i].bus)
                                      << QString::number(camerasList[i].device)
                                      << QString::number(motorsList[i].vendor)
                                      << QString::number(motorsList[i].product)
                                      << QString::number(motorsList[i].bus)
                                      << QString::number(motorsList[i].device));
        _apps[i]->waitForStarted(-1);

        _openniInterfaces[i] = new OpenNIApplicationInterface((i == 0) ? DBUS_SERVICE_NAME_1 : DBUS_SERVICE_NAME_2, DBUS_ROOT_OBJECT,
                                                              QDBusConnection::sessionBus(), this);

        qDebug() << qPrintable(tr("OpenNI Application n°%1 started !").arg(i));
    }
}

void OpenNIProcessWorker::setMotorAngle(const int kinectID, const int angle)
{
    _openniInterfaces[kinectID]->moveToAngle(angle);
}

void OpenNIProcessWorker::requestStop()
{
    _stopped = true;
}

void OpenNIProcessWorker::setAngleBetweenSensors(bool clockwise)
{
    _clockwise = clockwise ? 1 : -1;
}

int OpenNIProcessWorker::orientationValue()
{
    return cameraInformations().averageRotation;
}

int OpenNIProcessWorker::walkSpeedValue()
{
    return cameraInformations().averageWalkSpeed;
}

int OpenNIProcessWorker::specialCode()
{
    return _specialCode;
}

OpenNIUtil::CameraInformations OpenNIProcessWorker::cameraInformations()
{
    if(_infoMemories[0] == nullptr || (_useTwoSensors && _infoMemories[1] == nullptr)
       || !_infoMemories[0]->isAttached() || (_useTwoSensors && !_infoMemories[1]->isAttached())
       || _openniInterfaces[0] == nullptr || (_useTwoSensors && _openniInterfaces[1] == nullptr)
       || !_started[0] || (_useTwoSensors && !_started[1]))
        return OpenNIUtil::createInvalidCamInfo();

    OpenNIUtil::CameraInformations camInfo1;

    _infoMemories[0]->lock();
    void *mem = _infoMemories[0]->data();
    camInfo1 = *(static_cast<OpenNIUtil::CameraInformations *>(mem));
    _infoMemories[0]->unlock();

    camInfo1.averageRotation = camInfo1.user.rotation;
    camInfo1.averageWalkSpeed = camInfo1.user.walkSpeed;

    if(camInfo1.user.numberOfFramesWithoutMove > 10)
        _specialCode = 2;
    else
        _specialCode = 0;

    if(_useTwoSensors)
    {
        _infoMemories[1]->unlock();
        void *mem2 = _infoMemories[1]->data();
        OpenNIUtil::CameraInformations camInfo2 = *(static_cast<OpenNIUtil::CameraInformations *>(mem2));
        _infoMemories[1]->unlock();

        int averageRotation;
        int averageWalkSpeed;

        // This is the rotation of the sensor 2, projected into the sensor 1 image
        int projectedRotation2 = camInfo2.user.rotation + (90 * _clockwise);

        if(projectedRotation2 >= 360)
            projectedRotation2 -= 360;
        else if(projectedRotation2 < 0)
            projectedRotation2 = 360 + projectedRotation2;

        //
        // Mix rotation
        //

        if(camInfo1.user.rotation == -1)
            averageRotation = camInfo2.user.rotation;
        else if(camInfo2.user.rotation == -1)
            averageRotation = camInfo1.user.rotation;
        else
        {
            // Use coefficients in the average
            /*averageRotation = ((camInfo1.user.rotation * camInfo1.user.rotationConfidence)
                    + (projectedRotation2 * camInfo2.user.rotationConfidence))
                    / (camInfo1.user.rotationConfidence + camInfo2.user.rotationConfidence);*/
            if(camInfo1.user.rotationConfidence > camInfo2.user.rotationConfidence)
                averageRotation = camInfo1.user.rotation;
            else
                averageRotation = projectedRotation2;
        }

        // Smooth rotation
        if(_previousAverageRotation != -1)
        {
            if(std::abs(_previousAverageRotation - averageRotation) > 10)
            {
                if(_previousAverageRotation > averageRotation)
                    averageRotation = _previousAverageRotation - 10;
                else
                    averageRotation = _previousAverageRotation + 10;
            }
        }

        //
        // Mix walkSpeed
        //

        if(camInfo1.user.walkSpeed == -1)
            averageWalkSpeed = camInfo2.user.walkSpeed;
        else if(camInfo2.user.walkSpeed == -1)
            averageWalkSpeed = camInfo1.user.walkSpeed;
        else
        {
            averageWalkSpeed = ((camInfo1.user.walkSpeed * camInfo1.user.walkSpeedConfidence)
                    + (camInfo2.user.walkSpeed) * camInfo2.user.walkSpeedConfidence)
                    / (camInfo1.user.walkSpeedConfidence + camInfo2.user.walkSpeedConfidence);
        }

        camInfo1.hasSecondView = true;
        camInfo1.secondUser = camInfo2.user;
        camInfo1.secondRotationProjected = projectedRotation2;
        camInfo1.averageRotation = averageRotation;
        camInfo1.averageWalkSpeed = averageWalkSpeed;

        _previousAverageRotation = averageRotation;
    }

    return camInfo1;
}

OpenNIUtil::DepthMaps OpenNIProcessWorker::depthMaps()
{
    if(_depthMemories[0] == nullptr || (_useTwoSensors && _depthMemories[1] == nullptr)
       || !_depthMemories[0]->isAttached() || (_useTwoSensors && !_depthMemories[1]->isAttached())
       || _openniInterfaces[0] == nullptr || (_useTwoSensors && _openniInterfaces[1] == nullptr)
       || !_started[0] || (_useTwoSensors && !_started[1]))
        return OpenNIUtil::createInvalidDepthMaps();

    OpenNIUtil::DepthMaps depthMaps;

    for(int i=0 ; i<_sensorsCount; ++i)
    {
        _depthMemories[i]->lock();
        void *mem = _depthMemories[i]->data();
        XnDepthPixel * pixels = static_cast<XnDepthPixel*>(mem);
        _depthMemories[i]->unlock();

        if(pixels == nullptr)
            depthMaps.invalid = true;

        if(i==0)
            depthMaps.depthData = pixels;
        else
            depthMaps.secondDepthData = pixels;
    }

    return depthMaps;

}

// Private slots
void OpenNIProcessWorker::newLogData(int id)
{
    QString data;
    data = _apps[id]->readAllStandardError();

    // This is a special commad used to tell when the app is ready to send datas
    if(data.startsWith("*****started*****"))
    {
        _started[id] = true;
        return;
    }

    if(!_stopped)
        qDebug() << qPrintable(data);
}
