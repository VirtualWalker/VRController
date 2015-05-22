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

#include <QString>
#include <QDebug>

#include <chrono>
#include <cmath>

// These defines are used to avoid to much code repetition
#define CHECK_ERROR(retVal, what)                                                                                                                  \
    if(retVal != XN_STATUS_OK)                                                                                                                     \
    {                                                                                                                                              \
        qCritical() << qPrintable(QObject::tr("%1 failed: %2", "%1 is what failed and %2 is the error from OpenNI SDK.").arg(what).arg(xnGetStatusString(retVal)));   \
        return retVal;                                                                                                                             \
    }

#define GET_OPENNI_APP(cookie, app)                                                    \
    app = static_cast<OpenNIApplication*>(cookie);                                     \
    if(app == nullptr)                                                                 \
    {                                                                                  \
        qCritical() << qPrintable(QObject::tr("Cannot get the OpenNI application."));  \
        return;                                                                        \
    }

//
// Internal callbacks, only called by OpenNI internaly
// The OpenNIApplicationr is represented by the *cookie pointer
//

void XN_CALLBACK_TYPE newUserCallback(xn::UserGenerator& /*generator*/, XnUserID userID, void* cookie)
{
    // New user found, start calibration
    OpenNIApplication *app;
    GET_OPENNI_APP(cookie, app);
    qDebug() << qPrintable(QObject::tr("New user: %1").arg(userID));
    app->startCalibration(userID);
}

void XN_CALLBACK_TYPE lostUserCallback(xn::UserGenerator& /*generator*/, XnUserID userID, void* cookie)
{
    OpenNIApplication *app;
    GET_OPENNI_APP(cookie, app);
    qDebug() << qPrintable(QObject::tr("Lost user: %1").arg(userID));
}

void XN_CALLBACK_TYPE calibrationStartCallback(xn::SkeletonCapability& /*capability*/, XnUserID userID, void* cookie)
{
    OpenNIApplication *app;
    GET_OPENNI_APP(cookie, app);
    qDebug() << qPrintable(QObject::tr("Calibration started for user: %1").arg(userID));
}

void XN_CALLBACK_TYPE calibrationEndCallback(xn::SkeletonCapability& /*capability*/, XnUserID userID, XnCalibrationStatus calibrationStatus, void* cookie)
{
    OpenNIApplication *app;
    GET_OPENNI_APP(cookie, app);
    // Check the status
    if(calibrationStatus == XN_CALIBRATION_STATUS_OK)
    {
        // Calibration succeeded
        qDebug() << qPrintable(QObject::tr("Calibration complete, start tracking user %1").arg(userID));
        app->startTracking(userID);
    }
    else
    {
        // Calibration failed
        qWarning() << qPrintable(QObject::tr("Calibration failed for user: %1").arg(userID));
        if(calibrationStatus == XN_CALIBRATION_STATUS_MANUAL_ABORT)
            qWarning() << qPrintable(QObject::tr("Manual abort occured, stop attempting to calibrate !"));
        else // Restart the calibration process
            app->startCalibration(userID);
    }
}

OpenNIApplication::OpenNIApplication(int frequency, USBDevicePath camPath, USBDevicePath motorPath, QObject *parent) : QObject(parent)
{
    _frequency = frequency;

    _cameraPath = camPath;
    _motorPath = motorPath;
}

OpenNIApplication::~OpenNIApplication()
{
    cleanup();
    _kinectUSB->setLight(USBController::LightType::LED_BLINK_GREEN);
    _kinectUSB->moveToAngle(0);
    _kinectUSB->deleteLater();
}

// Private
void OpenNIApplication::cleanup()
{
    _depthGenerator.Release();
    _userGenerator.Release();

    _context.Release();

    _mutex.lock();
    _stopped = true;
    _mutex.unlock();
}

// Getters
bool OpenNIApplication::isInitialized() const
{
    return _init;
}

bool OpenNIApplication::isStarted()
{
    _mutex.lock();
    const bool started = _started;
    _mutex.unlock();
    return started;
}

bool OpenNIApplication::isStopped()
{
    _mutex.lock();
    const bool stopped = _stopped;
    _mutex.unlock();
    return stopped;
}

OpenNIUtil::CameraInformations OpenNIApplication::lastCamInfo()
{
    _mutex.lock();
    OpenNIUtil::CameraInformations camInfo = _lastCamInfo;
    _mutex.unlock();
    return camInfo;
}

// Setter
void OpenNIApplication::requestStop()
{
    _mutex.lock();
    _stopRequested = true;
    _mutex.unlock();

    // Directly call cleanup() method if the application is not yet started
    if(!isStarted())
        cleanup();
}

// Init the app
XnStatus OpenNIApplication::init()
{
    if(_init)
    {
        qCritical() << qPrintable(tr("OpenNI already initialized !"));
        return 1;
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

    // Check if the specified device in constructor exists
    bool found = false;
    for(xn::NodeInfoList::Iterator it = devicesList.Begin(); it != devicesList.End(); ++it)
    {
        // A sensor was found, add to the list
        if((*it).GetCreationInfo() == _cameraPath.toString())
        {
            xn::NodeInfo nodeInfo = *it;
            found = true;

            qDebug() << qPrintable(tr("Creating device: %1").arg(_cameraPath.toString()));

            status = _context.CreateProductionTree(nodeInfo, _device);
            CHECK_ERROR(status, tr("Create device", "on error"));

            // Create the query for the current node
            xn::Query query;
            query.AddNeededNode(nodeInfo.GetInstanceName());

            // Create the depth generator
            status = _context.CreateAnyProductionTree(XN_NODE_TYPE_DEPTH, &query, _depthGenerator);
            CHECK_ERROR(status, tr("Create depth generator", "on error"));

            // Create the user generator
            status = _context.CreateAnyProductionTree(XN_NODE_TYPE_USER, &query, _userGenerator);
            CHECK_ERROR(status, tr("Create user generator", "on error"));

            // Register all users callbacks
            status = _userGenerator.RegisterUserCallbacks(&newUserCallback, &lostUserCallback, this, _userCBHandler);
            CHECK_ERROR(status, tr("Register to user callbacks", "on error"));
            status = _userGenerator.GetSkeletonCap().RegisterToCalibrationStart(&calibrationStartCallback, this, _calibrationStartCBHandler);
            CHECK_ERROR(status, tr("Register to calibration start", "on error"));
            status = _userGenerator.GetSkeletonCap().RegisterToCalibrationComplete(&calibrationEndCallback, this, _calibrationEndCBHandler);
            CHECK_ERROR(status, tr("Register to calibration complete", "on error"));

            // Check if the user generator support skeleton
            if(!_userGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
            {
                qCritical() << qPrintable(tr("Supplied user generator doesn't support skeleton capability."));
                return 20;
            }
            // Check if the user generator need a pose for skeleton detection
            if(_userGenerator.GetSkeletonCap().NeedPoseForCalibration())
            {
                qCritical() << qPrintable(tr("Pose calibration required but not supported by this program."));
                return 30;
            }

            // Set the skeleton profile
            _userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

            // Create the usb controller
            _kinectUSB = new USBController(this);
            _kinectUSB->init(_motorPath);
            // Move the kinect to the angle 0 at startup
            if(_kinectUSB->initialized())
            {
                _kinectUSB->moveToAngle(0);
                _kinectUSB->setLight(USBController::LightType::LED_GREEN);
            }

            // Exit the for loop
            break;
        }
    }

    if(!found)
    {
        qCritical() << qPrintable(tr("The specified device doesn't exist !"));
        return 4;
    }

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
        _mutex.lock();
        if(_stopRequested)
        {
            _mutex.unlock();
            cleanup();
            break;
        }
        _mutex.unlock();

        // Get the previous user data
        _mutex.lock();
        const OpenNIUtil::User previousUser = _lastCamInfo.user;
        _mutex.unlock();
        OpenNIUtil::CameraInformations camInfo;

        _context.WaitAnyUpdateAll();

        camInfo.depthData = const_cast<XnDepthPixel *>(_depthGenerator.GetDepthMap());

        // Try to get 5 users, but only save the first tracked
        XnUInt16 usersCount = 5;
        XnUserID usersArray[usersCount];
        _userGenerator.GetUsers(usersArray, usersCount);
        // Get the first tracked user
        XnUserID firstTrackingID = 0;
        for(XnUserID id : usersArray)
        {
            if(_userGenerator.GetSkeletonCap().IsTracking(id))
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

            user.leftPart.hip = createJoint(XN_SKEL_LEFT_HIP, user.id);
            user.leftPart.knee = createJoint(XN_SKEL_LEFT_KNEE, user.id);
            user.leftPart.foot = createJoint(XN_SKEL_LEFT_FOOT, user.id);
            user.leftPart.shoulder = createJoint(XN_SKEL_LEFT_SHOULDER, user.id);
            user.rightPart.hip = createJoint(XN_SKEL_RIGHT_HIP, user.id);
            user.rightPart.knee = createJoint(XN_SKEL_RIGHT_KNEE, user.id);
            user.rightPart.foot = createJoint(XN_SKEL_RIGHT_FOOT, user.id);
            user.rightPart.shoulder = createJoint(XN_SKEL_RIGHT_SHOULDER, user.id);

            user.torsoJoint = createJoint(XN_SKEL_TORSO, user.id);

            user.previousLeftPart = previousUser.leftPart;
            user.previousRightPart = previousUser.rightPart;

            OpenNIUtil::rotationForUser(_frequency, previousUser.rotation, &user);

            // Only compute the walk speed if we are not in the first frame
            if(!firstLoop)
            {
                user.walkSpeed = OpenNIUtil::walkSpeedForUser(_frequency, user, previousUser.timestamp, previousUser.walkSpeed);
                if(previousUser.walkSpeed != -1 && previousUser.walkSpeed <= MIN_COMPUTED_WALKSPEED)
                    user.numberOfFramesWithoutMove = previousUser.numberOfFramesWithoutMove + 1;
                else
                    user.numberOfFramesWithoutMove = 0;
            }
        }
        else
        {
            user.id = 0;
            user.isTracking = false;
        }

        _mutex.lock();
        _lastCamInfo = camInfo;
        _lastCamInfo.user = user;
        _mutex.unlock();

        if(firstLoop)
        {
            _mutex.lock();
            firstLoop = false;
            _started = true;
            _mutex.unlock();
        }
    }

    return status;
}

OpenNIUtil::Joint OpenNIApplication::createJoint(const XnSkeletonJoint jointType, const XnUserID userID)
{
    OpenNIUtil::Joint joint;
    joint.type = jointType;
    joint.isActive = _userGenerator.GetSkeletonCap().IsJointActive(jointType);
    if(joint.isActive)
    {
        // Get the position info
        _userGenerator.GetSkeletonCap().GetSkeletonJointPosition(userID, jointType, joint.info);
        // Get the projectives positions
        _depthGenerator.ConvertRealWorldToProjective(1, &joint.info.position, &joint.projectivePos);
    }
    return joint;
}

void OpenNIApplication::moveToAngle(const int angle)
{
    if(_kinectUSB != nullptr && _kinectUSB->initialized())
        _kinectUSB->moveToAngle(angle);
}

void OpenNIApplication::setLight(const USBController::LightType type)
{
    if(_kinectUSB != nullptr && _kinectUSB->initialized())
        _kinectUSB->setLight(type);
}

XnStatus OpenNIApplication::startCalibration(const XnUserID userID)
{
    _mutex.lock();
    if(_started)
    {
        _mutex.unlock();
        return _userGenerator.GetSkeletonCap().RequestCalibration(userID, TRUE);
    }
    _mutex.unlock();
    return 100;
}

XnStatus OpenNIApplication::startTracking(const XnUserID userID)
{
    _mutex.lock();
    if(_started)
    {
        _mutex.unlock();
        return _userGenerator.GetSkeletonCap().StartTracking(userID);
    }
    _mutex.unlock();
    return 101;
}

