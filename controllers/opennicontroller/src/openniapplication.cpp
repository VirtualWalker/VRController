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

#include <QTemporaryFile>
#include <QFile>
#include <QString>
#include <QDir>
#include <chrono>

// These defines are used to avoid to much code repetition
#define CHECK_ERROR(retVal, what)                                                                                                                  \
    if(retVal != XN_STATUS_OK)                                                                                                                     \
    {                                                                                                                                              \
        qCritical() << qPrintable(QObject::tr("%1 failed: %2", "%1 is what failed and %2 is the error from OpenNI SDK.").arg(what).arg(retVal));   \
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
// The OpenNIApplication is represented by the *cookie pointer
//

void XN_CALLBACK_TYPE newUserCallback(xn::UserGenerator& /*generator*/, XnUserID userID, void* cookie)
{
    qDebug() << qPrintable(QObject::tr("New user: %1", "%1: user ID").arg(userID));
    // New user found, start calibration
    OpenNIApplication *app;
    GET_OPENNI_APP(cookie, app);
    app->startCalibration(userID);
}

void XN_CALLBACK_TYPE lostUserCallback(xn::UserGenerator& /*generator*/, XnUserID userID, void* cookie)
{
    qDebug() << qPrintable(QObject::tr("Lost user: %1", "%1: user ID").arg(userID));
    OpenNIApplication *app;
    GET_OPENNI_APP(cookie, app);
    if(app->useAKinect())
        app->setLight(USBController::LightType::LED_BLINK_YELLOW);
}

void XN_CALLBACK_TYPE calibrationStartCallback(xn::SkeletonCapability& /*capability*/, XnUserID userID, void* /*cookie*/)
{
    qDebug() << qPrintable(QObject::tr("Calibration started for user: %1", "%1: user ID").arg(userID));
}

void XN_CALLBACK_TYPE calibrationEndCallback(xn::SkeletonCapability& /*capability*/, XnUserID userID, XnCalibrationStatus calibrationStatus, void* cookie)
{
    OpenNIApplication *app;
    GET_OPENNI_APP(cookie, app);
    // Check the status
    if(calibrationStatus == XN_CALIBRATION_STATUS_OK)
    {
        // Calibration succeeded
        qDebug() << qPrintable(QObject::tr("Calibration complete, start tracking user %1", "%1: user ID").arg(userID));
        app->startTracking(userID);
    }
    else
    {
        // Calibration failed
        qWarning() << qPrintable(QObject::tr("Calibration failed for user: %1", "%1: user ID").arg(userID));
        if(calibrationStatus == XN_CALIBRATION_STATUS_MANUAL_ABORT)
            qWarning() << qPrintable(QObject::tr("Manual abort occured, stop attempting to calibrate !"));
        else // Restart the calibration process
            app->startCalibration(userID);
    }
}

void XN_CALLBACK_TYPE calibrationInProgressCallback(xn::SkeletonCapability& /*capability*/, XnUserID userID, XnCalibrationStatus calibrationStatus, void* cookie)
{
    OpenNIApplication *app;
    GET_OPENNI_APP(cookie, app);
    // Add calibration status to the list (auto inserted if not exist)
    app->setCalibrationStatus(userID, calibrationStatus);
}

OpenNIApplication::OpenNIApplication(bool useAKinect, QObject *parent) : QObject(parent)
{
    _useAKinect = useAKinect;
    if(_useAKinect)
        qDebug() << qPrintable(tr("The application will assume that a kinect sensor is used !"));
}

OpenNIApplication::~OpenNIApplication()
{
    cleanup();
    if(_useAKinect)
    {
        _kinectUSB->setLight(USBController::LightType::LED_OFF);
        _kinectUSB->deleteLater();
    }
}

// Private
void OpenNIApplication::cleanup()
{
    qDebug() << qPrintable(QObject::tr("Stopping the OpenNI frame loop ..."));

    _xmlScriptNode.Release();
    _depthGenerator.Release();
    _userGenerator.Release();
    _context.Release();

    _stoppedMutex.lock();
    _stopped = true;
    _stoppedMutex.unlock();
}

// Getters
bool OpenNIApplication::initialized()
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

// Init the app
XnStatus OpenNIApplication::init()
{
    if(_init)
    {
        qCritical() << qPrintable(tr("OpenNI already initialized !"));
        return 1;
    }

    // Initialize the motor
    if(_useAKinect)
    {
        // Init the motor controller
        _kinectUSB = new KinectUSBController(this);
        _kinectUSB->init();
        // Move the kinect to the angle 0 at startup
        if(_kinectUSB->initialized())
        {
            _kinectUSB->moveToAngle(0);
            _kinectUSB->setLight(USBController::LightType::LED_BLINK_RED_YELLOW);
        }
    }

    qDebug() << qPrintable(tr("Initializing OpenNI ..."));

    XnStatus status = XN_STATUS_OK;

    // Copy the config file from resources to disk
    QTemporaryFile *tempFile = QTemporaryFile::createNativeFile(QStringLiteral(":/opennicontroller/OpenNIKinectConfig.xml"));

    // Read the XML file
    xn::EnumerationErrors errors;
    status = _context.InitFromXmlFile(tempFile->fileName().toStdString().c_str(), _xmlScriptNode, &errors);
    // Check for errors
    if(status == XN_STATUS_NO_NODE_PRESENT)
    {
        // Print the errors
        XnChar strError[1024];
        errors.ToString(strError, 1024);
        qDebug() << strError;
        return status;
    }
    else CHECK_ERROR(status, tr("Open XML file", "on error"));

    delete tempFile;
    tempFile = nullptr;

    // Load the depth generator
    status = _context.FindExistingNode(XN_NODE_TYPE_DEPTH, _depthGenerator);
    CHECK_ERROR(status, tr("Find depth generator", "on error"));

    // Load the user generator
    status = _context.FindExistingNode(XN_NODE_TYPE_USER, _userGenerator);
    // Create a default one if nothing specified
    if(status != XN_STATUS_OK)
    {
        status = _userGenerator.Create(_context);
        CHECK_ERROR(status, tr("Find user generator", "on error"));
    }

    // Register callbacks
    status = _userGenerator.RegisterUserCallbacks(&newUserCallback, &lostUserCallback, this, _userCBHandler);
    CHECK_ERROR(status, tr("Register to user callbacks", "on error"));
    status = _userGenerator.GetSkeletonCap().RegisterToCalibrationStart(&calibrationStartCallback, this, _calibrationStartCBHandler);
    CHECK_ERROR(status, tr("Register to calibration start", "on error"));
    status = _userGenerator.GetSkeletonCap().RegisterToCalibrationComplete(&calibrationEndCallback, this, _calibrationEndCBHandler);
    CHECK_ERROR(status, tr("Register to calibration complete", "on error"));
    status = _userGenerator.GetSkeletonCap().RegisterToCalibrationInProgress(&calibrationInProgressCallback, this, _calibrationInProgressCBHandler);
    CHECK_ERROR(status, tr("Register to calibration in progress", "on error"));

    // Check if the user generator support skeleton
    if(!_userGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
    {
        qCritical() << qPrintable(tr("Supplied user generator doesn't support skeleton capability."));
        return 2;
    }
    // Check if the user generator need a pose for skeleton detection
    if(_userGenerator.GetSkeletonCap().NeedPoseForCalibration())
    {
        qCritical() << qPrintable(tr("Pose calibration required but not supported by this program."));
        return 3;
    }

    // Set the skeleton profile (only get the lower part of the body)
    _userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);


    if(_useAKinect)
        _kinectUSB->setLight(USBController::LightType::LED_BLINK_YELLOW);

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

        const XnDepthPixel* depthData = _depthGenerator.GetDepthMap();

        // Process the frames
        _context.WaitOneUpdateAll(_depthGenerator);

        depthData = _depthGenerator.GetDepthMap();

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

        // Get the previous user data
        _lastCamInfoMutex.lock();
        const OpenNIUtil::User previousUser = _lastCamInfo.user;
        _lastCamInfoMutex.unlock();

        // Set the user
        OpenNIUtil::User user;
        if(firstTrackingID != 0)
        {
            user.id = firstTrackingID;
            user.isTracking = true;

            user.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

            user.leftLeg.hip = createJoint(XN_SKEL_LEFT_HIP, user.id);
            user.leftLeg.knee = createJoint(XN_SKEL_LEFT_KNEE, user.id);
            user.leftLeg.foot = createJoint(XN_SKEL_LEFT_FOOT, user.id);
            user.rightLeg.hip = createJoint(XN_SKEL_RIGHT_HIP, user.id);
            user.rightLeg.knee = createJoint(XN_SKEL_RIGHT_KNEE, user.id);
            user.rightLeg.foot = createJoint(XN_SKEL_RIGHT_FOOT, user.id);

            user.previousLeftLeg = previousUser.leftLeg;
            user.previousRightLeg = previousUser.rightLeg;

            // To compute the rotation of the player, we use the hip joints
            user.rotation = static_cast<int>(OpenNIUtil::rotationFrom2Joints(user.rightLeg.hip, user.leftLeg.hip, previousUser.rotation, &user.rotationConfidence));

            // Only compute the walk speed if we are not in the first frame
            if(!firstLoop)
                user.walkSpeed = OpenNIUtil::walkSpeedForUser(user, previousUser.timestamp, previousUser.walkSpeed, &user.walkSpeedConfidence);
        }
        // If no user user is tracked, get the first in the list
        else if(usersCount >= 1)
        {
            user.id = usersArray[0];
            user.isTracking = false;
        }
        else
        {
            user.id = 0;
            user.isTracking = false;
        }

        _lastCamInfoMutex.lock();
        // Get previous values before
        _lastCamInfo = OpenNIUtil::CameraInformations();
        _lastCamInfo.depthData = depthData;
        _lastCamInfo.user = user;

        // Emit some signals
        emit camInfoChanged();
        if(previousUser.rotation != _lastCamInfo.user.rotation)
            emit orientationChanged(_lastCamInfo.user.rotation);
        if(previousUser.walkSpeed != _lastCamInfo.user.walkSpeed)
            emit walkSpeedChanged(_lastCamInfo.user.walkSpeed);

        _lastCamInfoMutex.unlock();

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
        _depthGenerator.ConvertRealWorldToProjective(1, &joint.info.position, &joint.projectivePosition);
    }
    return joint;
}

void OpenNIApplication::moveToAngle(const int angle)
{
    if(_useAKinect && _kinectUSB != nullptr && _kinectUSB->initialized())
        _kinectUSB->moveToAngle(angle);
    else
        qWarning() << qPrintable(tr("Trying to use a Kinect functionnality without enabling the support !"));
}

void OpenNIApplication::increaseAngle()
{
    if(_useAKinect && _kinectUSB != nullptr && _kinectUSB->initialized())
        _kinectUSB->increaseAngle();
    else
        qWarning() << qPrintable(tr("Trying to use a Kinect functionnality without enabling the support !"));
}

void OpenNIApplication::decreaseAngle()
{
    if(_useAKinect && _kinectUSB != nullptr && _kinectUSB->initialized())
        _kinectUSB->decreaseAngle();
    else
        qWarning() << qPrintable(tr("Trying to use a Kinect functionnality without enabling the support !"));
}

void OpenNIApplication::resetAngle()
{
    moveToAngle(0);
}

void OpenNIApplication::setLight(const USBController::LightType type)
{
    if(_useAKinect && _kinectUSB != nullptr && _kinectUSB->initialized())
        _kinectUSB->setLight(type);
    else
        qWarning() << qPrintable(tr("Trying to use a Kinect functionnality without enabling the support !"));
}

XnStatus OpenNIApplication::startCalibration(const XnUserID userID)
{
    _startedMutex.lock();
    if(_started)
    {
        _startedMutex.unlock();
        return _userGenerator.GetSkeletonCap().RequestCalibration(userID, TRUE);
    }
    _startedMutex.unlock();
    return 20;
}

XnStatus OpenNIApplication::startTracking(const XnUserID userID)
{
    _startedMutex.lock();
    if(_started)
    {
        _startedMutex.unlock();

        if(_useAKinect)
            _kinectUSB->setLight(USBController::LightType::LED_BLINK_GREEN);

        return _userGenerator.GetSkeletonCap().StartTracking(userID);
    }
    _startedMutex.unlock();
    return 21;
}

void OpenNIApplication::setCalibrationStatus(const XnUserID userID, XnCalibrationStatus calibStatus)
{
    _startedMutex.lock();
    if(_started)
    {
        _startedMutex.unlock();
        _calibrationStatus[userID] = calibStatus;
    }
    _startedMutex.unlock();
}



