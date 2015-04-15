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

#include "opencvutil.h"
#include "controllercommon.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <stdint.h>

// Use 10000 in the ratio since we don't want to see after 10 meters
#define DEPTH_IMAGE_RATIO (256.0f / 10000.0f)

cv::Point3f OpenCVUtil::pointToCV(const XnVector3D pt)
{
    return cv::Point3f(pt.X, pt.Y, pt.Z);
}

cv::Point2i OpenCVUtil::pointTo2DCV(const XnVector3D pt, const int offsetX, const int offsetY, const int res)
{
    return cv::Point2i(offsetX + pt.X*res, offsetY + pt.Y*res);
}

void OpenCVUtil::drawJoint(cv::Mat& image, const OpenNIUtil::Joint joint, const cv::Scalar color,
                           const int offsetX, const int offsetY, const int res)
{
    if(OpenNIUtil::isJointAcceptable(joint))
        cv::circle(image, OpenCVUtil::pointTo2DCV(joint.projectivePos, offsetX, offsetY, res),
                   6*res, color, CV_FILLED);
}

void OpenCVUtil::drawLimb(cv::Mat& image, const OpenNIUtil::Joint joint1, const OpenNIUtil::Joint joint2, const cv::Scalar color,
                          const int offsetX, const int offsetY, const int res)
{
    if(OpenNIUtil::isJointAcceptable(joint1) && OpenNIUtil::isJointAcceptable(joint2))
        cv::line(image, OpenCVUtil::pointTo2DCV(joint1.projectivePos, offsetX, offsetY, res),
                 OpenCVUtil::pointTo2DCV(joint2.projectivePos, offsetX, offsetY, res), color, 2*res);
}

void OpenCVUtil::drawJointsOfUser(cv::Mat &image, const OpenNIUtil::User user, const cv::Scalar rightColor,
                                  const cv::Scalar leftColor, const cv::Scalar centerColor,
                                  const int offsetX, const int offsetY, const int res)
{
    drawJoint(image, user.leftPart.hip, leftColor, offsetX, offsetY, res);
    drawJoint(image, user.leftPart.knee, leftColor, offsetX, offsetY, res);
    drawJoint(image, user.leftPart.foot, leftColor, offsetX, offsetY, res);
    drawJoint(image, user.leftPart.shoulder, leftColor, offsetX, offsetY, res);

    drawJoint(image, user.rightPart.hip, rightColor, offsetX, offsetY, res);
    drawJoint(image, user.rightPart.knee, rightColor, offsetX, offsetY, res);
    drawJoint(image, user.rightPart.foot, rightColor, offsetX, offsetY, res);
    drawJoint(image, user.rightPart.shoulder, rightColor, offsetX, offsetY, res);

    drawJoint(image, user.torsoJoint, centerColor, offsetX, offsetY, res);
}

void OpenCVUtil::drawLimbsOfUsers(cv::Mat &image, const OpenNIUtil::User user, const cv::Scalar color,
                                  const int offsetX, const int offsetY, const int res)
{
    drawLimb(image, user.leftPart.hip, user.leftPart.knee, color, offsetX, offsetY, res);
    drawLimb(image, user.leftPart.knee, user.leftPart.foot, color, offsetX, offsetY, res);

    drawLimb(image, user.rightPart.hip, user.rightPart.knee, color, offsetX, offsetY, res);
    drawLimb(image, user.rightPart.knee, user.rightPart.foot, color, offsetX, offsetY, res);

    drawLimb(image, user.leftPart.hip, user.rightPart.hip, color, offsetX, offsetY, res);

    drawLimb(image, user.leftPart.hip, user.torsoJoint, color, offsetX, offsetY, res);
    drawLimb(image, user.torsoJoint, user.leftPart.shoulder, color, offsetX, offsetY, res);

    drawLimb(image, user.rightPart.hip, user.torsoJoint, color, offsetX, offsetY, res);
    drawLimb(image, user.torsoJoint, user.rightPart.shoulder, color, offsetX, offsetY, res);

    drawLimb(image, user.leftPart.shoulder, user.rightPart.shoulder, color, offsetX, offsetY, res);
}

void OpenCVUtil::drawTextCentered(cv::Mat& image, const std::string& text, const cv::Point& centerPoint,
                      const int& fontFace, const double& fontScale, const cv::Scalar& color,
                      const int& thickness)
{
    const cv::Size textRect = cv::getTextSize(text, fontFace, fontScale, thickness, nullptr);
    // Compute the text origin (to have the text at the center)
    const cv::Point origin(centerPoint.x - (textRect.width/2), centerPoint.y + (textRect.height/2));
    cv::putText(image, text, origin, fontFace, fontScale, color, thickness);
}

// The image type must be CV_8UC3
void OpenCVUtil::drawDepthMap(cv::Mat &image, XnDepthPixel* depthMap,
                              const int startX, const int startY, const int res)
{
    const int depthMapWidth = 640 * res;
    const int depthMapHeight = 480 * res;

    XnDepthPixel* depthData = depthMap;

    uint8_t* pixelPtr = (uint8_t*)image.data;
    for(int r=startY; r < startY + depthMapHeight; r += res)
    {
        for(int c=startX; c < startX + depthMapWidth; c += res)
        {
            const uint16_t realColor = (*depthData) * DEPTH_IMAGE_RATIO;
            const uint8_t color = realColor > UINT8_MAX ? UINT8_MAX : realColor;

            for(int i=0; i < res; ++i)
            {
                for(int j=0; j < res; ++j)
                {
                    pixelPtr[(r+i)*image.cols*3 + (c+j)*3] = color;
                    pixelPtr[(r+i)*image.cols*3 + (c+j)*3 + 1] = color;
                    pixelPtr[(r+i)*image.cols*3 + (c+j)*3 + 2] = color;
                }
            }
            depthData++;
        }
    }
}

#define IMG_RES 2
#define LEFT_PART_WIDTH (640*IMG_RES)
#define RIGHT_PART_WIDTH (400*IMG_RES)
#define IMG_WIDTH (LEFT_PART_WIDTH + RIGHT_PART_WIDTH)
#define IMG_HEIGHT (480*IMG_RES)

#define FONT_FACE cv::FONT_HERSHEY_DUPLEX
#define ROT_FONTSCALE 4
#define ROT_THICKNESS 4

#define WALK_LINE_START (LEFT_PART_WIDTH + 100)
#define WALK_LINE_END (IMG_WIDTH - 100)
#define WALK_LINE_WIDTH (WALK_LINE_END - WALK_LINE_START)
#define WALK_LINE_HEIGHT (IMG_HEIGHT - 80)
#define WALK_LINE_THICKNESS 20
#define WALK_LINE_RATIO (WALK_LINE_WIDTH / WALK_SPEED_POSSIBILITIES)

#define COLOR_1 CV_RGB(0, 255, 255)
#define COLOR_2 CV_RGB(72, 209, 204)
#define COLOR_3 CV_RGB(0, 128, 128)

static int guiDialOrientation = 0;
static int guiWalkSpeed = 0;
static int guiWalkSpeedIncrease = 5;

// There is two parts in the image:
// - left part with the depth data and skeleton
// - right part with some informations
cv::Mat OpenCVUtil::drawOpenNIData(OpenNIUtil::CameraInformations camInfo, OpenNIUtil::DepthMaps depthMaps)
{
    const cv::Scalar backColor = CV_RGB(10,10,10);
    cv::Mat outputMat = cv::Mat(IMG_HEIGHT, IMG_WIDTH, CV_8UC3, backColor);

    //
    // Left part
    // (with all depth maps)
    //

    // Draw a large depth map if there is only one sensor
    if(!camInfo.hasSecondView)
    {
        drawDepthMap(outputMat, depthMaps.depthData, 0, 0, IMG_RES);

        drawLimbsOfUsers(outputMat, camInfo.user, CV_RGB(0, 180, 0), 0, 0, IMG_RES);
        drawJointsOfUser(outputMat, camInfo.user, CV_RGB(255, 0, 0), CV_RGB(0, 0, 255), CV_RGB(120, 0, 0), 0, 0, IMG_RES);
    }
    else
    {
        // First sensor
        drawDepthMap(outputMat, depthMaps.depthData, 0, 0, 1);

        drawLimbsOfUsers(outputMat, camInfo.user, CV_RGB(0, 180, 0), 0, 0, 1);
        drawJointsOfUser(outputMat, camInfo.user, CV_RGB(255, 0, 0), CV_RGB(120, 0, 0), CV_RGB(80, 0, 0), 0, 0, 1);

        std::string rotText1 = std::to_string(camInfo.user.rotation);
        drawTextCentered(outputMat, rotText1, cv::Point(640 + 640/2, 480/2), FONT_FACE, ROT_FONTSCALE, COLOR_1, ROT_THICKNESS);

        // Second sensor
        drawDepthMap(outputMat, depthMaps.secondDepthData, 640, 480, 1);

        drawLimbsOfUsers(outputMat, camInfo.secondUser, CV_RGB(180, 180, 0), 640, 480, 1);
        drawJointsOfUser(outputMat, camInfo.secondUser, CV_RGB(0, 0, 255), CV_RGB(0, 0, 120), CV_RGB(0, 0, 80), 640, 480, 1);

        std::string rotText2 = std::to_string(camInfo.secondUser.rotation);
        rotText2 += "/";
        rotText2 += std::to_string(camInfo.secondRotationProjected);
        drawTextCentered(outputMat, rotText2, cv::Point(640/2, 480 + 480/2), FONT_FACE, 3, COLOR_1, ROT_THICKNESS);
    }

    //
    // Right part
    // The GUI of this part is inspired by this picture :
    // <http://i.ytimg.com/vi/YAXsZphpiu8/maxresdefault.jpg>
    //

    //
    // Draw circle for the rotation

    std::string textRotation = "???";

    // Check if we have the rotation
    const int camInfoRotation = camInfo.hasSecondView ? camInfo.averageRotation : camInfo.user.rotation;
    if(camInfoRotation != -1)
    {
        guiDialOrientation = camInfoRotation;
        textRotation = std::to_string(camInfoRotation);
    }
    else
    {
        guiDialOrientation += 5;
        if(guiDialOrientation >= 360)
            guiDialOrientation = 360 - guiDialOrientation;
    }

    // Set the dial start and dial end
    const int dialStart = guiDialOrientation - 25;
    const int dialEnd = guiDialOrientation + 25;    

    const cv::Point circleCenter((RIGHT_PART_WIDTH/2) + LEFT_PART_WIDTH, 280);
    cv::circle(outputMat, circleCenter, 100*2, COLOR_1, 5*2);
    cv::circle(outputMat, circleCenter, 80*2, COLOR_3, 15*2);
    cv::ellipse(outputMat, circleCenter, cv::Size(94*2,94*2), 0, dialStart, dialEnd, COLOR_2, -1);
    cv::ellipse(outputMat, circleCenter, cv::Size(91*2,91*2), 0, dialStart - 3, dialEnd + 3, backColor, -1);
    cv::ellipse(outputMat, circleCenter, cv::Size(89*2,89*2), 0, dialStart, dialEnd, COLOR_2, -1);
    cv::ellipse(outputMat, circleCenter, cv::Size(65*2,65*2), 0, dialStart - 3, dialEnd + 3, backColor, -1);

    // Print rotation in the center of the circle
    drawTextCentered(outputMat, textRotation, circleCenter, FONT_FACE, ROT_FONTSCALE, COLOR_1, ROT_THICKNESS);

    //
    // Draw the walk speed

    cv::line(outputMat, cv::Point(WALK_LINE_START, WALK_LINE_HEIGHT), cv::Point(WALK_LINE_END, WALK_LINE_HEIGHT), COLOR_1, WALK_LINE_THICKNESS);
    cv::line(outputMat, cv::Point(LEFT_PART_WIDTH + 30, WALK_LINE_HEIGHT), cv::Point(LEFT_PART_WIDTH + 70, WALK_LINE_HEIGHT), COLOR_3, WALK_LINE_THICKNESS);
    cv::line(outputMat, cv::Point(IMG_WIDTH - 30, WALK_LINE_HEIGHT), cv::Point(IMG_WIDTH - 70, WALK_LINE_HEIGHT), COLOR_3, WALK_LINE_THICKNESS);

    std::string textWalkSpeed = "??";

    // Draw vertical line and walk speed value
    const int camInfoWalkSpeed = camInfo.hasSecondView ? camInfo.averageWalkSpeed : camInfo.user.walkSpeed;
    if(camInfoWalkSpeed != -1)
    {
        guiWalkSpeed = camInfoWalkSpeed;
        textWalkSpeed = std::to_string(camInfoWalkSpeed);
    }
    else
    {
        guiWalkSpeed += guiWalkSpeedIncrease;
        if(guiWalkSpeed > MAX_WALK_SPEED || guiWalkSpeed < MIN_WALK_SPEED)
        {
            guiWalkSpeedIncrease *= -1;
            guiWalkSpeed += guiWalkSpeedIncrease;
        }
    }

    const int currentWalkSpeedX = WALK_LINE_START + (guiWalkSpeed * WALK_LINE_RATIO);
    cv::line(outputMat, cv::Point(currentWalkSpeedX, WALK_LINE_HEIGHT - 30), cv::Point(currentWalkSpeedX, WALK_LINE_HEIGHT - 60), COLOR_2, 10);

    drawTextCentered(outputMat, textWalkSpeed, cv::Point(currentWalkSpeedX, WALK_LINE_HEIGHT - 100), FONT_FACE, 2, COLOR_2, 2);

    return outputMat;
}

