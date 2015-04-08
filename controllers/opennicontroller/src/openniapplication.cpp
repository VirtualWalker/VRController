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

#include "openniapplication.h"

#include "core/utility.h"

#include <QString>
#include <QDebug>
#include <QProcess>
#include <QByteArray>

#include <chrono>

// Simple struct that contains the application and the sensor id
// This is used in user callbacks to retrive the caller of the callback
struct OpenNIApplicationWrapper
{
    OpenNIApplication *app;
    int callerID;
};


// These defines are used to avoid to much code repetition
#define CHECK_ERROR(retVal, what)                                                                                                                  \
    if(retVal != XN_STATUS_OK)                                                                                                                     \
    {                                                                                                                                              \
        qCritical() << qPrintable(QObject::tr("%1 failed: %2", "%1 is what failed and %2 is the error from OpenNI SDK.").arg(what).arg(xnGetStatusString(retVal)));   \
        return retVal;                                                                                                                             \
    }

#define GET_OPENNI_WRAPPER(cookie, wrapper)                                            \
    wrapper = static_cast<OpenNIApplicationWrapper*>(cookie);                          \
    if(wrapper == nullptr)                                                             \
    {                                                                                  \
        qCritical() << qPrintable(QObject::tr("Cannot get the OpenNI application."));  \
        return;                                                                        \
    }

//
// Internal callbacks, only called by OpenNI internaly
// The OpenNIApplicationWrapper is represented by the *cookie pointer
//

void XN_CALLBACK_TYPE newUserCallback(xn::UserGenerator& /*generator*/, XnUserID userID, void* cookie)
{
    // New user found, start calibration
    OpenNIApplicationWrapper *wrapper;
    GET_OPENNI_WRAPPER(cookie, wrapper);
    qDebug() << qPrintable(QObject::tr("New user: %1 (from sensor %2)").arg(userID).arg(wrapper->callerID));
    wrapper->app->startCalibration(userID, wrapper->callerID);
}

void XN_CALLBACK_TYPE lostUserCallback(xn::UserGenerator& /*generator*/, XnUserID userID, void* cookie)
{
    OpenNIApplicationWrapper *wrapper;
    GET_OPENNI_WRAPPER(cookie, wrapper);
    qDebug() << qPrintable(QObject::tr("Lost user: %1 (from sensor %2)").arg(userID).arg(wrapper->callerID));
}

void XN_CALLBACK_TYPE calibrationStartCallback(xn::SkeletonCapability& /*capability*/, XnUserID userID, void* cookie)
{
    OpenNIApplicationWrapper *wrapper;
    GET_OPENNI_WRAPPER(cookie, wrapper);
    qDebug() << qPrintable(QObject::tr("Calibration started for user: %1 (from sensor %2)").arg(userID).arg(wrapper->callerID));
}

void XN_CALLBACK_TYPE calibrationEndCallback(xn::SkeletonCapability& /*capability*/, XnUserID userID, XnCalibrationStatus calibrationStatus, void* cookie)
{
    OpenNIApplicationWrapper *wrapper;
    GET_OPENNI_WRAPPER(cookie, wrapper);
    // Check the status
    if(calibrationStatus == XN_CALIBRATION_STATUS_OK)
    {
        // Calibration succeeded
        qDebug() << qPrintable(QObject::tr("Calibration complete, start tracking user %1 (from sensor %2)").arg(userID).arg(wrapper->callerID));
        wrapper->app->startTracking(userID, wrapper->callerID);
    }
    else
    {
        // Calibration failed
        qWarning() << qPrintable(QObject::tr("Calibration failed for user: %1 (from sensor %2)").arg(userID).arg(wrapper->callerID));
        if(calibrationStatus == XN_CALIBRATION_STATUS_MANUAL_ABORT)
            qWarning() << qPrintable(QObject::tr("Manual abort occured, stop attempting to calibrate !"));
        else // Restart the calibration process
            wrapper->app->startCalibration(userID, wrapper->callerID);
    }
}

OpenNIApplication::OpenNIApplication(bool useAKinect, bool useTwoSensors, QObject *parent) : QObject(parent)
{
    _useAKinect = useAKinect;
    if(_useAKinect)
        qDebug() << qPrintable(tr("The application will assume that a kinect sensor is used !"));

    if(useTwoSensors)
    {
        qDebug() << qPrintable(tr("The application will use two sensors !"));
        qDebug() << qPrintable(tr("The angle between the sensor 2 and the sensor 1 must be of 90°."));
        qDebug() << qPrintable(tr("You can tell if this angle is clockwise or counterclockwise inside the GUI."));
    }

    _sensorsCount = useTwoSensors ? 2 : 1;
}

OpenNIApplication::~OpenNIApplication()
{
    cleanup();
    if(_useAKinect)
    {
        for(int i=0; i<_sensorsList.size(); ++i)
        {
            _sensorsList[i].kinectUSB->setLight(USBController::LightType::LED_OFF);
            _sensorsList[i].kinectUSB->moveToAngle(0);
            _sensorsList[i].kinectUSB->deleteLater();
        }
    }
}

// Private
void OpenNIApplication::cleanup()
{
    qDebug() << qPrintable(QObject::tr("Stopping the OpenNI frame loop ..."));

    for(Sensor &sensor : _sensorsList)
    {
        sensor.depthGenerator.Release();
        sensor.userGenerator.Release();
    }

    _context.Release();

    _stoppedMutex.lock();
    _stopped = true;
    _stoppedMutex.unlock();
}

// Getters
bool OpenNIApplication::initialized() const
{
    return _init;
}

bool OpenNIApplication::started()
{
    _startedMutex.lock();
    const bool started = _started;
    _startedMutex.unlock();
    return started;
}

bool OpenNIApplication::stopped()
{
    _stoppedMutex.lock();
    const bool stopped = _stopped;
    _stoppedMutex.unlock();
    return stopped;
}

bool OpenNIApplication::useAKinect() const
{
    return _useAKinect;
}

int OpenNIApplication::sensorsCount() const
{
    return _sensorsCount;
}

OpenNIUtil::CameraInformations OpenNIApplication::lastCamInfo()
{
    _lastCamInfoMutex.lock();
    const OpenNIUtil::CameraInformations camInfo = _lastCamInfo;
    _lastCamInfoMutex.unlock();
    return camInfo;
}

int OpenNIApplication::lastOrientation()
{
    _lastCamInfoMutex.lock();
    const int orientation = _lastCamInfo.user.rotation;
    _lastCamInfoMutex.unlock();
    return orientation;
}

int OpenNIApplication::lastWalkSpeed()
{
    _lastCamInfoMutex.lock();
    const int walkSpeed = _lastCamInfo.user.walkSpeed;
    _lastCamInfoMutex.unlock();
    return walkSpeed;
}

// Setter
void OpenNIApplication::requestStop()
{
    _stopRequestedMutex.lock();
    _stopRequested = true;
    _stopRequestedMutex.unlock();

    // Directly call cleanup() method if the application is not yet started
    if(!started())
        cleanup();
}

void OpenNIApplication::setAngleBetweenSensors(bool clockwise)
{
    _clockwiseMutex.lock();
    _clockwise = clockwise ? 1 : -1;
    _clockwiseMutex.unlock();
}

// Init the app
XnStatus OpenNIApplication::init()
{
    if(_init)
    {
        qCritical() << qPrintable(tr("OpenNI already initialized !"));
        return 1;
    }
    
    if(!_useAKinect && _sensorsCount == 2)
    {
        qCritical() << qPrintable(tr("You must use Kinect sensors if you want to use two sensors !"));
        return 2;
    }

    qDebug() << qPrintable(tr("Initializing OpenNI ..."));

    XnStatus status = XN_STATUS_OK;

    // Init the context
    status = _context.Init();
    CHECK_ERROR(status, tr("Context Init", "on error"));

    // List all sensors
    xn::NodeInfoList devicesList;
    status = _context.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, nullptr, devicesList);
    CHECK_ERROR(status, tr("Search available devices", "on error"));
    
    // List all available devices using the "lsusb" command
    QProcess lsusbProcess;
    lsusbProcess.start("lsusb");
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
        if(line.contains("045e") && line.contains("Microsoft Corp. Xbox NUI"))
        {
            // Cameras
            if(line.contains("ID 045e:02ae"))
            {
                camerasList.append(USBDevicePath({KINECT_VENDOR_ID, KINECT_CAMERA_PRODUCT_ID,
                                                 line.mid(4, 3).toInt(), line.mid(15, 3).toInt()}));
            }
            // Motors
            else if(line.contains("ID 045e:02b0"))
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
        return 3;
    }

    // Search sensors from the lsusb output in the OpenNI devices list
    for(int i=0; i<_sensorsCount; ++i)
    {
        for(xn::NodeInfoList::Iterator it = devicesList.Begin(); it != devicesList.End(); ++it)
        {
            // A sensor was found, add to the list
            if((*it).GetCreationInfo() == camerasList[i].toString())
            {
                Sensor sensor;
                sensor.nodeInfo = *it;
                sensor.cameraPath = camerasList[i];
                sensor.motorPath = motorsList[i];
                _sensorsList.append(sensor);
                break;
            }
        }
    }

    // Check if we have the correct number of sensors
    if(_sensorsCount != _sensorsList.size())
    {
        qCritical() << qPrintable(tr("There is not enough connected sensors !"));
        return 4;
    }

    // Get only required devices and create all generators
    for(int i=0; i<_sensorsCount ; ++i)
    {   
        qDebug() << qPrintable(tr("Creating device: %1").arg(_sensorsList[i].cameraPath.toString()));

        status = _context.CreateProductionTree(_sensorsList[i].nodeInfo, _sensorsList[i].device);
        CHECK_ERROR(status, tr("Create device (sensor %1)", "on error").arg(i));

        // Create the query for the current node
        xn::Query query;
        query.AddNeededNode(_sensorsList[i].nodeInfo.GetInstanceName());

        // Create the depth generator
        status = _context.CreateAnyProductionTree(XN_NODE_TYPE_DEPTH, &query, _sensorsList[i].depthGenerator);
        CHECK_ERROR(status, tr("Create depth generator (sensor %1)", "on error").arg(i));

        // Create the user generator
        status = _context.CreateAnyProductionTree(XN_NODE_TYPE_USER, &query, _sensorsList[i].userGenerator);
        CHECK_ERROR(status, tr("Create user generator (sensor %1)", "on error").arg(i));

        // Register all users callbacks
        status = _sensorsList[i].userGenerator.RegisterUserCallbacks(&newUserCallback, &lostUserCallback, new OpenNIApplicationWrapper({this, i}), _userCBHandler);
        CHECK_ERROR(status, tr("Register to user callbacks (sensor %1)", "on error").arg(i));
        status = _sensorsList[i].userGenerator.GetSkeletonCap().RegisterToCalibrationStart(&calibrationStartCallback, new OpenNIApplicationWrapper({this, i}), _calibrationStartCBHandler);
        CHECK_ERROR(status, tr("Register to calibration start (sensor %1)", "on error").arg(i));
        status = _sensorsList[i].userGenerator.GetSkeletonCap().RegisterToCalibrationComplete(&calibrationEndCallback, new OpenNIApplicationWrapper({this, i}), _calibrationEndCBHandler);
        CHECK_ERROR(status, tr("Register to calibration complete (sensor %1)", "on error").arg(i));

        // Check if the user generator support skeleton
        if(!_sensorsList[i].userGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
        {
            qCritical() << qPrintable(tr("Supplied user generator doesn't support skeleton capability (sensor %1).").arg(i));
            return 20;
        }
        // Check if the user generator need a pose for skeleton detection
        if(_sensorsList[i].userGenerator.GetSkeletonCap().NeedPoseForCalibration())
        {
            qCritical() << qPrintable(tr("Pose calibration required but not supported by this program (sensor %1).").arg(i));
            return 30;
        }

        // Set the skeleton profile
        _sensorsList[i].userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

        // Create the usb controller
        if(_useAKinect)
        {
            // Init the motor controller
            _sensorsList[i].kinectUSB = new USBController(this);
            _sensorsList[i].kinectUSB->init(_sensorsList[i].motorPath);
            // Move the kinect to the angle 0 at startup
            if(_sensorsList[i].kinectUSB->initialized())
            {
                _sensorsList[i].kinectUSB->moveToAngle(0);
                if(i == 0)
                    _sensorsList[i].kinectUSB->setLight(USBController::LightType::LED_BLINK_GREEN);
                else
                    _sensorsList[i].kinectUSB->setLight(USBController::LightType::LED_BLINK_RED_YELLOW);
            }
        }
    }

    // Summary all devices in the console
    qDebug() << qPrintable(tr("Number of connected devices: %1").arg(_sensorsCount));
    for(int i=0; i<_sensorsCount; ++i)
        qDebug() << qPrintable(tr("Sensor n°%1: Blink %2, %3", "%2 is the color, %3 is the connection string")
                               .arg(i).arg((i == 0) ? tr("Green") : tr("Red-Yellow")).arg(_sensorsList[i].cameraPath.toString()));

    _init = true;
    return status;
}

XnStatus OpenNIApplication::start()
{
    if(!_init)
    {
        qCritical() << qPrintable(tr("The application is not initilized, can't start !"));
        return 5;
    }

    qDebug() << qPrintable(tr("Starting OpenNI main loop ..."));

    const XnStatus status = _context.StartGeneratingAll();
    CHECK_ERROR(status, tr("Start Generating", "on error"));

    // Start the frame loop
    bool firstLoop = true;

    while(true)
    {
        _stopRequestedMutex.lock();
        if(_stopRequested)
        {
            _stopRequestedMutex.unlock();
            cleanup();
            break;
        }
        _stopRequestedMutex.unlock();

        //_context.WaitAnyUpdateAll();

        // Get data for each sensor
        for(int i=0; i<_sensorsCount; ++i)
        {
            // Get the previous user data
            const OpenNIUtil::User previousUser = _sensorsList[i].camInfo.user;

            _sensorsList[i].camInfo = OpenNIUtil::CameraInformations();
            _sensorsList[i].camInfo.depthData = _sensorsList[i].depthGenerator.GetDepthMap();

            _context.WaitAnyUpdateAll();

            // Retrieve depth map
            _sensorsList[i].camInfo.depthData = _sensorsList[i].depthGenerator.GetDepthMap();

            // Try to get 5 users, but only save the first who is tracked
            XnUInt16 usersCount = 5;
            XnUserID usersArray[usersCount];
            _sensorsList[i].userGenerator.GetUsers(usersArray, usersCount);
            // Get the first tracked user
            XnUserID firstTrackingID = 0;
            for(XnUserID id : usersArray)
            {
                if(_sensorsList[i].userGenerator.GetSkeletonCap().IsTracking(id))
                {
                    firstTrackingID = id;
                    break;
                }
            }

            // Set the current user
            OpenNIUtil::User user;
            if(firstTrackingID != 0)
            {
                user.id = firstTrackingID;
                user.isTracking = true;

                user.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

                user.leftLeg.hip = createJoint(XN_SKEL_LEFT_HIP, user.id, i);
                user.leftLeg.knee = createJoint(XN_SKEL_LEFT_KNEE, user.id, i);
                user.leftLeg.foot = createJoint(XN_SKEL_LEFT_FOOT, user.id, i);
                user.rightLeg.hip = createJoint(XN_SKEL_RIGHT_HIP, user.id, i);
                user.rightLeg.knee = createJoint(XN_SKEL_RIGHT_KNEE, user.id, i);
                user.rightLeg.foot = createJoint(XN_SKEL_RIGHT_FOOT, user.id, i);

                user.previousLeftLeg = previousUser.leftLeg;
                user.previousRightLeg = previousUser.rightLeg;

                // To compute the rotation of the player, we use the hip joints
                user.rotation = static_cast<int>(OpenNIUtil::rotationFrom2Joints(user.rightLeg.hip, user.leftLeg.hip, previousUser.rotation, &user.rotationConfidence));

                // Only compute the walk speed if we are not in the first frame
                if(!firstLoop)
                    user.walkSpeed = OpenNIUtil::walkSpeedForUser(user, previousUser.timestamp, previousUser.walkSpeed, &user.walkSpeedConfidence);
            }
            else
            {
                user.id = 0;
                user.isTracking = false;
            }

            _sensorsList[i].camInfo.user = user;
        }

        // Now, we can mix data from the two sensors
        if(_sensorsCount == 1)
        {
            _lastCamInfoMutex.lock();
            _lastCamInfo = _sensorsList[0].camInfo;
            _lastCamInfoMutex.unlock();
        }
        else
        {
            // This is the rotation of the sensor 2, projected into the sensor 1 image
            _clockwiseMutex.lock();
            int projectedRotation2 = _sensorsList[1].camInfo.user.rotation + (90 * _clockwise);
            _clockwiseMutex.unlock();

            if(projectedRotation2 >= 360)
                projectedRotation2 -= 360;
            else if(projectedRotation2 < 0)
                projectedRotation2 = 360 + projectedRotation2;

            int mixedRotation = (_sensorsList[0].camInfo.user.rotation + projectedRotation2) / 2;
            int mixedWalkSpeed = (_sensorsList[0].camInfo.user.walkSpeed + _sensorsList[1].camInfo.user.walkSpeed) / 2;

            _lastCamInfoMutex.lock();
            _lastCamInfo = OpenNIUtil::CameraInformations();
            _lastCamInfo.depthData = _sensorsList[0].camInfo.depthData;
            _lastCamInfo.user = _sensorsList[0].camInfo.user;
            _lastCamInfo.hasSecondView = true;
            _lastCamInfo.secondUser = _sensorsList[1].camInfo.user;
            _lastCamInfo.mixedRotation = mixedRotation;
            _lastCamInfo.mixedWalkSpeed = mixedWalkSpeed;
            _lastCamInfoMutex.unlock();
        }

        if(firstLoop)
        {
            _startedMutex.lock();
            firstLoop = false;
            _started = true;
            _startedMutex.unlock();
        }
    }

    return status;
}

OpenNIUtil::Joint OpenNIApplication::createJoint(const XnSkeletonJoint jointType, const XnUserID userID, const int sensorID)
{
    OpenNIUtil::Joint joint;
    joint.type = jointType;
    joint.isActive = _sensorsList[sensorID].userGenerator.GetSkeletonCap().IsJointActive(jointType);
    if(joint.isActive)
    {
        // Get the position info
        _sensorsList[sensorID].userGenerator.GetSkeletonCap().GetSkeletonJointPosition(userID, jointType, joint.info);
        // Get the projectives positions
        _sensorsList[sensorID].depthGenerator.ConvertRealWorldToProjective(1, &joint.info.position, &joint.projectivePosition);
    }
    return joint;
}

void OpenNIApplication::moveToAngle(const int kinectID, const int angle)
{
    if(_useAKinect && _sensorsList[kinectID].kinectUSB != nullptr && _sensorsList[kinectID].kinectUSB->initialized())
        _sensorsList[kinectID].kinectUSB->moveToAngle(angle);
    else
        qWarning() << qPrintable(tr("Trying to use a Kinect functionnality without enabling the support !"));
}

void OpenNIApplication::setLight(const int kinectID, const USBController::LightType type)
{
    if(_useAKinect && _sensorsList[kinectID].kinectUSB != nullptr && _sensorsList[kinectID].kinectUSB->initialized())
        _sensorsList[kinectID].kinectUSB->setLight(type);
    else
        qWarning() << qPrintable(tr("Trying to use a Kinect functionnality without enabling the support !"));
}

XnStatus OpenNIApplication::startCalibration(const XnUserID userID, const int sensorID)
{
    _startedMutex.lock();
    if(_started)
    {
        _startedMutex.unlock();
        return _sensorsList[sensorID].userGenerator.GetSkeletonCap().RequestCalibration(userID, TRUE);
    }
    _startedMutex.unlock();
    return 100;
}

XnStatus OpenNIApplication::startTracking(const XnUserID userID, const int sensorID)
{
    _startedMutex.lock();
    if(_started)
    {
        _startedMutex.unlock();
        return _sensorsList[sensorID].userGenerator.GetSkeletonCap().StartTracking(userID);
    }
    _startedMutex.unlock();
    return 101;
}

