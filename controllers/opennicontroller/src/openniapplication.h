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

#ifndef OPENNIAPPLICATION_H
#define OPENNIAPPLICATION_H

#include <ni/XnOpenNI.h>
#include <ni/XnCodecIDs.h>
#include <map>
#include <mutex>

#include <QObject>
#include <QList>

#include "openniutil.h"
#include "usbcontroller.h"

// Represent a Depth-Sensor
struct Sensor
{
    xn::ProductionNode device;
    xn::NodeInfo nodeInfo = xn::NodeInfo(nullptr);

    USBDevicePath cameraPath;
    USBDevicePath motorPath;

    // Only used if we use a Kinect
    USBController *kinectUSB;

    xn::DepthGenerator depthGenerator;
    xn::UserGenerator userGenerator;

    OpenNIUtil::CameraInformations camInfo;
};

// This class is a bridge between the program and the OpenNI API.
// When started, you can retrieve the last informations using lastCamInfo()
class OpenNIApplication: public QObject
{
        Q_OBJECT
    public:
        // Nothing is created in the constructor.
        // Please call init() to start the process
        OpenNIApplication(bool useAKinect = false, bool useTwoSensors = false, QObject *parent = nullptr);
        ~OpenNIApplication();

        // Check if the app is initialized
        bool initialized() const;

        // Check if the app is started
        bool started();

        // Check if the app is stopped
        bool stopped();

        // Request the stop of the app
        void requestStop();

        // Initialise the app
        // Must be call only one time
        XnStatus init();

        // Start the application
        // Must be call just after init()
        // This function enter in an infinite loop
        // To exit, please call the requestStop() function
        XnStatus start();

        // Don't call these functions directly, they are usually called from the Callbacks methods
        XnStatus startCalibration(const XnUserID userID, const int sensorID);
        XnStatus startTracking(const XnUserID userID, const int sensorID);

        // Getters
        OpenNIUtil::CameraInformations lastCamInfo();
        int lastOrientation();
        int lastWalkSpeed();

        bool useAKinect() const;
        int sensorsCount() const;

    public slots:
        // These functions are only available if you are using a Kinect sensor
        void moveToAngle(const int kinectID, const int angle);
        void setLight(const int kinectID, const USBController::LightType type);

        void setAngleBetweenSensors(bool clockwise);

    private:

        bool _init = false;

        bool _started = false;
        std::mutex _startedMutex;

        bool _stopped = false;
        std::mutex _stoppedMutex;

        // Tell if we are using a kinect sensor or not
        bool _useAKinect;

        // Sensors count (1 or 2)
        int _sensorsCount;
        QList<Sensor> _sensorsList;

        // Tell if the angle between two kinects is clockwise or counterclockwise
        // 1 --> clockwise
        // -1 --> counterclockwise
        int _clockwise = 1;
        std::mutex _clockwiseMutex;

        xn::Context _context;

        // Generators
        xn::DepthGenerator _depthGenerator;
        xn::UserGenerator _userGenerator;

        // Callbacks handler
        XnCallbackHandle _userCBHandler;
        XnCallbackHandle _calibrationStartCBHandler;
        XnCallbackHandle _calibrationEndCBHandler;

        bool _stopRequested = false;
        std::mutex _stopRequestedMutex;

        OpenNIUtil::CameraInformations _lastCamInfo;
        std::mutex _lastCamInfoMutex;

        OpenNIUtil::Joint createJoint(const XnSkeletonJoint jointType, const XnUserID userID, const int sensorID);

        // Cleanup all OpenNI objects
        void cleanup();
};

#endif // OPENNIAPPLICATION_H


