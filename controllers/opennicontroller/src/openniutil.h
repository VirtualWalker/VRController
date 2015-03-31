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

#ifndef OPENNIUTILS_H
#define OPENNIUTILS_H

#include <ni/XnTypes.h>
#include <cmath>

#include "controllercommon.h"

// Contains some structures for OpenNI data
namespace OpenNIUtil
{
    struct Joint
    {
        XnSkeletonJoint type;
        // Check if the join is active
        bool isActive;
        XnSkeletonJointPosition info;
        XnPoint3D projectivePosition;
    };

    struct Leg
    {
        Joint hip;
        Joint knee;
        Joint foot;
    };

    struct User
    {
        XnUserID id;
        bool isTracking;

        // The timestamp when this object was generated
        // Represent the time in milliseconds since the epoch time
        int64_t timestamp;

        // Current legs informations
        Leg leftLeg;
        Leg rightLeg;

        // Legs informations at the previous frame
        Leg previousLeftLeg;
        Leg previousRightLeg;

        int rotation = -1;
        int walkSpeed = -1;
    };

    struct CameraInformations
    {
        // The depth map (values are in mm)
        const XnDepthPixel* depthData;
        User user;
    };

    inline bool isJointAcceptable(const Joint joint)
    {
        return joint.isActive && joint.info.fConfidence >= 0.6;
    }

    // The previous rotation parameter is used to avoid big differences between two rotation
    inline float rotationFrom2Joints(const Joint rightJoint, const Joint leftJoint, float previousRotation)
    {
        if(isJointAcceptable(rightJoint) && isJointAcceptable(leftJoint))
        {
            const float angle = std::atan(std::abs(rightJoint.info.position.Z - leftJoint.info.position.Z)
                                          / std::abs(rightJoint.info.position.X - leftJoint.info.position.X)) * RAD2DEG;

            float rotation = -1.0f;

            // 0
            if(rightJoint.info.position.Z == leftJoint.info.position.Z
               && rightJoint.info.position.X > leftJoint.info.position.X)
                rotation = 0.0f;
            // 90
            else if(rightJoint.info.position.X == leftJoint.info.position.X
                    && rightJoint.info.position.Z > leftJoint.info.position.Z)
                rotation = 90.0f;
            // 180
            else if(rightJoint.info.position.Z == leftJoint.info.position.Z
                    && rightJoint.info.position.X < leftJoint.info.position.X)
                rotation = 180.0f;
            // 270
            else if(rightJoint.info.position.X == leftJoint.info.position.X
                    && rightJoint.info.position.Z < leftJoint.info.position.Z)
                rotation = 90.0f;

            // 0 - 180
            else if(rightJoint.info.position.Z < leftJoint.info.position.Z)
            {
                // 0 - 90
                if(rightJoint.info.position.X > leftJoint.info.position.X)
                    rotation = angle;
                // 90 - 180
                else
                    rotation = 180.0f - angle;
            }
            // 180 - 360
            else
            {
                // 180 - 270
                if(rightJoint.info.position.X < leftJoint.info.position.X)
                    rotation = 180.0f + angle;
                // 270 - 360
                else
                    rotation =  360.0f - angle;
            }

            //
            // Smooth the rotation
            //

            if(previousRotation != -1.0f)
            {
                // If the difference of rotation is higher than 180 degree, we consider
                // that we are move from the 360 deg to the 0 deg side
                // In this case, add 360 degrees to the lower value
                if(std::abs(rotation - previousRotation) > 180.0f)
                {
                    if(rotation < previousRotation)
                        rotation += 360.0f;
                    else
                        previousRotation += 360.0f;
                }

                const float diffRotation = rotation - previousRotation;
                if(std::abs(diffRotation) > 10.0f)
                {
                    // If new rotation is higher
                    if(diffRotation > 0.0f)
                    {
                        rotation = previousRotation + 10.0f;
                    }
                    // If new rotation is lower
                    else
                    {
                        rotation = previousRotation - 10.0f;
                    }
                }
            }

            return rotation;
        }

        return -1.0f;
    }

    inline int walkSpeedForUser(const User& user, const int64_t& previousTimestamp, const int& previousSpeed)
    {
        // Compute the x and z diff for the right and left foot
        float rdx = 0;
        float rdz = 0;
        float ldx = 0;
        float ldz = 0;

        // Tell if we are not able to compute the speed
        bool cantCompute = true;

        if(isJointAcceptable(user.rightLeg.foot) && isJointAcceptable(user.previousRightLeg.foot))
        {
            rdx = user.previousRightLeg.foot.info.position.X - user.rightLeg.foot.info.position.X;
            rdz = user.previousRightLeg.foot.info.position.Z - user.rightLeg.foot.info.position.Z;

            if(isJointAcceptable(user.leftLeg.foot) && isJointAcceptable(user.previousLeftLeg.foot))
            {
                ldx = user.previousLeftLeg.foot.info.position.X - user.leftLeg.foot.info.position.X;
                ldz = user.previousLeftLeg.foot.info.position.Z - user.leftLeg.foot.info.position.Z;
                cantCompute = false;
            }
        }

        if(cantCompute)
            return -1;

        const float rightDiff = std::sqrt(std::pow(rdx, 2.0) + std::pow(rdz, 2.0));
        const float leftDiff = std::sqrt(std::pow(ldx, 2.0) + std::pow(ldz, 2.0));

        // Compute the average of diff (in mm)
        const float diff = (rightDiff + leftDiff) / 2.0;

        // Compute diff of timestamp
        const int64_t diffTime = user.timestamp - previousTimestamp;

        // Now compute the speed in cm/s
        int speed = static_cast<int>((diff * 0.1) / ((double)(diffTime) * 0.001));

        // Smooth the value depending on the last one
        if(previousSpeed != -1)
        {
            if(std::abs(speed - previousSpeed) > 10)
            {
                if(speed < previousSpeed)
                    speed = previousSpeed - 10;
                else
                    speed = previousSpeed + 10;
            }
        }

        return speed;
    }
}

#endif // OPENNIUTILS_H

