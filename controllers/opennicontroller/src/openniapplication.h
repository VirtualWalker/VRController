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
#include <ni/XnCppWrapper.h>
#include <ni/XnPropNames.h>
#include <map>
#include <mutex>

#include <QDebug>
#include <QObject>

#include "openniutil.h"

// This class is a bridge between the program and the OpenNI API.
// When started, you can retrieve the last informations using lastCamInfo()
class OpenNIApplication: public QObject
{
        Q_OBJECT
    public:
        // Nothing is created in the constructor.
        // Please call init() to start the process
        OpenNIApplication(QObject *parent = nullptr): QObject(parent) {}
        ~OpenNIApplication();

        // Check if the app is initialized
        bool initialized();

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
        XnStatus startCalibration(const XnUserID userID);
        XnStatus startTracking(const XnUserID userID);
        void setCalibrationStatus(const XnUserID userID, XnCalibrationStatus calibStatus);

        // Getters
        OpenNIUtil::CameraInformations lastCamInfo();
        int lastOrientation();
        // TODO: implement, for now always return 0
        int lastWalkSpeed();

    signals:

        void camInfoChanged();
        void orientationChanged(int newOrientation);
        void walkSpeedChanged(int newWalkSpeed);

    private:

        bool _init = false;

        bool _started = false;
        std::mutex _startedMutex;

        bool _stopped = false;
        std::mutex _stoppedMutex;

        xn::Context _context;
        xn::ScriptNode _xmlScriptNode;

        // Generators
        xn::DepthGenerator _depthGenerator;
        xn::UserGenerator _userGenerator;

        // Callbacks handler
        XnCallbackHandle _userCBHandler;
        XnCallbackHandle _calibrationStartCBHandler;
        XnCallbackHandle _calibrationEndCBHandler;
        XnCallbackHandle _calibrationInProgressCBHandler;

        // Array of all calibrations status
        std::map<XnUserID, XnCalibrationStatus> _calibrationStatus;

        bool _stopRequested = false;
        std::mutex _stopRequestedMutex;

        OpenNIUtil::CameraInformations _lastCamInfo;
        std::mutex _lastCamInfoMutex;

        OpenNIUtil::Joint createJoint(const XnSkeletonJoint jointType, const XnUserID userID);

        // Cleanup all OpenNI objects
        void cleanup();
};

#endif // OPENNIAPPLICATION_H


