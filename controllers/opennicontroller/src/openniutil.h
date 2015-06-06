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

#include <QVector>
#include <QDebug>

#include "controllercommon.h"

#define DEPTH_MAP_LENGTH (640*480)
#define MIN_COMPUTED_WALKSPEED 40

// Contains some structures for OpenNI data
namespace OpenNIUtil
{
    struct Joint
    {
        XnSkeletonJoint type;
        // Check if the join is active
        bool isActive = false;
        XnSkeletonJointPosition info;
        XnPoint3D projectivePos;
    };

    struct BodyPart
    {
        Joint hip;
        Joint knee;
        Joint foot;

        Joint shoulder;
    };

    // Contains all informations about the user
    struct User
    {
        XnUserID id;
        bool isTracking = false;

        // The timestamp when this object was generated
        // Represent the time in milliseconds since the epoch time
        int64_t timestamp;

        Joint torsoJoint;

        // Current body informations
        BodyPart leftPart;
        BodyPart rightPart;

        // Legs informations at the previous frame
        BodyPart previousLeftPart;
        BodyPart previousRightPart;

        int rotation = -1;
        int walkSpeed = -1;

        // Summary the number of frames since the last move
        int numberOfFramesWithoutMove = 0;
    };

    // Contains all data from the OpenNI loop
    struct CameraInformations
    {
        User user;

        // The depth map (values are in mm)
        XnDepthPixel *depthData = nullptr;

        bool invalid = false;

    };

    inline CameraInformations createInvalidCamInfo()
    {
        CameraInformations camInfo;
        camInfo.invalid = true;
        return camInfo;
    }

    inline bool isJointAcceptable(const Joint joint)
    {
        return joint.isActive && joint.info.fConfidence == 1.0f;
    }

    // Return the angle in range [0;360[
    inline float reduceAngle(const float angle)
    {
        if(angle >= 360.0f)
            return angle - 360.0f;
        else if(angle < 0.0f)
            return 360.0f + angle;
        return angle;
    }

    // Compute the mean angle using the circular average method
    // See <http://en.wikipedia.org/wiki/Mean_of_circular_quantities> for more informations
    inline float meanAngle(float *angles, const int size)
    {
        float x = 0;
        float y = 0;

        // used to check if all values are set to -1.0f
        bool allFalse = true;

        for(int i = 0; i < size; ++i)
        {
            if(angles[i] != -1.0f)
            {
                allFalse = false;
                x += std::cos(angles[i] * DEG2RAD);
                y += std::sin(angles[i] * DEG2RAD);
            }
        }

        return allFalse ? -1.0f : reduceAngle(std::atan2(y / size, x / size) * RAD2DEG);
    }

    // The previous rotation parameter is used to avoid big differences between two rotation
    inline float rotationFrom2Joints(const int frequency, const Joint rightJoint, const Joint leftJoint, float previousRotation)
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
                    && rightJoint.info.position.Z < leftJoint.info.position.Z)
                rotation = 90.0f;
            // 180
            else if(rightJoint.info.position.Z == leftJoint.info.position.Z
                    && rightJoint.info.position.X < leftJoint.info.position.X)
                rotation = 180.0f;
            // 270
            else if(rightJoint.info.position.X == leftJoint.info.position.X
                    && rightJoint.info.position.Z > leftJoint.info.position.Z)
                rotation = 270.0f;

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
                // If the difference of rotation is higher than 180°, we consider
                // that we move from the 360° to the 0° side
                // In this case, add 360° to the lower value
                if(std::abs(rotation - previousRotation) > 180.0f)
                {
                    if(rotation < previousRotation)
                        rotation += 360.0f;
                    else
                        previousRotation += 360.0f;
                }

                const float margin = 60.0f / (float)frequency;

                const float diffRotation = rotation - previousRotation;
                if(std::abs(diffRotation) > margin)
                {
                    // If new rotation is higher
                    if(diffRotation > 0.0f)
                    {
                        rotation = previousRotation + margin;
                    }
                    // If new rotation is lower
                    else
                    {
                        rotation = previousRotation - margin;
                    }
                }

                if(rotation >= 360.0f)
                    rotation -= 360.0f;
            }

            return rotation;
        }

        return -1.0f;
    }

    inline void rotationForUser(const int frequency, const int previousRotation, User* user)
    {
        float rotations[4] = {-1.0f, -1.0f, -1.0f, -1.0f};

        // right hip / left hip
        rotations[0] = rotationFrom2Joints(frequency, user->rightPart.hip, user->leftPart.hip,
                                            previousRotation);
        // right hip / torso
        rotations[1] = rotationFrom2Joints(frequency, user->rightPart.hip, user->torsoJoint,
                                            previousRotation);
        // torso / left hip
        rotations[2] = rotationFrom2Joints(frequency, user->torsoJoint, user->leftPart.hip,
                                            previousRotation);
        // right shoulder / left shoulder
        rotations[3] = rotationFrom2Joints(frequency, user->rightPart.shoulder, user->leftPart.shoulder,
                                            previousRotation);

        // Make an average
        user->rotation = static_cast<int>(meanAngle(rotations, 4));
    }

    inline int walkSpeedForUser(const int frequency, const User& user, const int64_t& previousTimestamp, const int& previousSpeed)
    {
        // Compute the x and z diff for the right and left foot
        float rdx = 0;
        float rdz = 0;
        float ldx = 0;
        float ldz = 0;

        // Tell if we are not able to compute the speed
        bool cantCompute = true;

        if(isJointAcceptable(user.rightPart.foot) && isJointAcceptable(user.previousRightPart.foot))
        {
            rdx = user.previousRightPart.foot.info.position.X - user.rightPart.foot.info.position.X;
            rdz = user.previousRightPart.foot.info.position.Z - user.rightPart.foot.info.position.Z;

            if(isJointAcceptable(user.leftPart.foot) && isJointAcceptable(user.previousLeftPart.foot))
            {
                ldx = user.previousLeftPart.foot.info.position.X - user.leftPart.foot.info.position.X;
                ldz = user.previousLeftPart.foot.info.position.Z - user.leftPart.foot.info.position.Z;
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
            const int margin = 100.0f / (float)frequency;

            if(std::abs(speed - previousSpeed) > margin)
            {
                if(speed < previousSpeed)
                    speed = previousSpeed - margin;
                else
                    speed = previousSpeed + margin;
            }
        }

        return speed;
    }
}

#endif // OPENNIUTILS_H

