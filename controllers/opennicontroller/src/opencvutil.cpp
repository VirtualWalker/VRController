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

cv::Point2i OpenCVUtil::pointTo2DCV(const XnVector3D pt, const int res)
{
    return cv::Point2i(pt.X*res, pt.Y*res);
}

void OpenCVUtil::drawJoint(cv::Mat& image, const OpenNIUtil::Joint joint, const cv::Scalar color, const int res)
{
    if(OpenNIUtil::isJointAcceptable(joint))
        cv::circle(image, OpenCVUtil::pointTo2DCV(joint.projectivePosition, res), 6*res, color, CV_FILLED);
}

void OpenCVUtil::drawLimb(cv::Mat& image, const OpenNIUtil::Joint joint1, const OpenNIUtil::Joint joint2, const cv::Scalar color, const int res)
{
    if(OpenNIUtil::isJointAcceptable(joint1) && OpenNIUtil::isJointAcceptable(joint2))
        cv::line(image, OpenCVUtil::pointTo2DCV(joint1.projectivePosition, res), OpenCVUtil::pointTo2DCV(joint2.projectivePosition, res), color, 2*res);
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

#define LEFT_PART_WIDTH (640*2)
#define RIGHT_PART_WIDTH (400*2)
#define IMG_WIDTH (LEFT_PART_WIDTH + RIGHT_PART_WIDTH)
#define IMG_HEIGHT (480*2)

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
cv::Mat OpenCVUtil::drawOpenNIData(OpenNIUtil::CameraInformations camInfo)
{
    const cv::Scalar backColor = CV_RGB(10,10,10);
    cv::Mat outputMat = cv::Mat(IMG_HEIGHT, IMG_WIDTH, CV_8UC3, backColor);
    uint8_t* pixelPtr = (uint8_t*)outputMat.data;

    //
    // Left part
    //

    // Draw depth map
    const XnDepthPixel* depthData = camInfo.depthData;
    for(int r=0; r < IMG_HEIGHT; r+=2)
    {
        for(int c=0; c < LEFT_PART_WIDTH; c+=2)
        {
            const uint16_t realColor = *depthData * DEPTH_IMAGE_RATIO;
            const uint8_t color = realColor > UINT8_MAX ? UINT8_MAX : realColor;

            pixelPtr[r*IMG_WIDTH*3 + c*3] = color;
            pixelPtr[r*IMG_WIDTH*3 + c*3 + 1] = color;
            pixelPtr[r*IMG_WIDTH*3 + c*3 + 2] = color;

            pixelPtr[r*IMG_WIDTH*3 + (c+1)*3] = color;
            pixelPtr[r*IMG_WIDTH*3 + (c+1)*3 + 1] = color;
            pixelPtr[r*IMG_WIDTH*3 + (c+1)*3 + 2] = color;

            pixelPtr[(r+1)*IMG_WIDTH*3 + c*3] = color;
            pixelPtr[(r+1)*IMG_WIDTH*3 + c*3 + 1] = color;
            pixelPtr[(r+1)*IMG_WIDTH*3 + c*3 + 2] = color;

            pixelPtr[(r+1)*IMG_WIDTH*3 + (c+1)*3] = color;
            pixelPtr[(r+1)*IMG_WIDTH*3 + (c+1)*3 + 1] = color;
            pixelPtr[(r+1)*IMG_WIDTH*3 + (c+1)*3 + 2] = color;

            depthData++;
        }
    }

    // Draw the limbs
    drawLimb(outputMat, camInfo.user.leftLeg.hip, camInfo.user.leftLeg.knee, CV_RGB(0, 255, 0));
    drawLimb(outputMat, camInfo.user.leftLeg.knee, camInfo.user.leftLeg.foot, CV_RGB(0, 255, 0));

    drawLimb(outputMat, camInfo.user.rightLeg.hip, camInfo.user.rightLeg.knee, CV_RGB(0, 255, 0));
    drawLimb(outputMat, camInfo.user.rightLeg.knee, camInfo.user.rightLeg.foot, CV_RGB(0, 255, 0));

    drawLimb(outputMat, camInfo.user.leftLeg.hip, camInfo.user.rightLeg.hip, CV_RGB(0, 255, 0));

    // Now draw the joints of the user
    drawJoint(outputMat, camInfo.user.leftLeg.hip, CV_RGB(128, 0, 0));
    drawJoint(outputMat, camInfo.user.leftLeg.knee, CV_RGB(255, 69, 0));
    drawJoint(outputMat, camInfo.user.leftLeg.foot, CV_RGB(218, 165, 32));

    drawJoint(outputMat, camInfo.user.rightLeg.hip, CV_RGB(0, 0, 255));
    drawJoint(outputMat, camInfo.user.rightLeg.knee, CV_RGB(30, 144, 255));
    drawJoint(outputMat, camInfo.user.rightLeg.foot, CV_RGB(127, 255, 212));

    //
    // Right part
    // The GUI of this part is inspired by this picture :
    // <http://i.ytimg.com/vi/YAXsZphpiu8/maxresdefault.jpg>
    //

    //
    // Draw circle for the rotation

    // Check if we have the rotation
    if(camInfo.user.rotation != -1)
        guiDialOrientation = camInfo.user.rotation;
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
    std::string textRotation = "???";
    if(camInfo.user.rotation != -1)
        textRotation = std::to_string(camInfo.user.rotation);
    drawTextCentered(outputMat, textRotation, circleCenter, FONT_FACE, ROT_FONTSCALE, COLOR_1, ROT_THICKNESS);

    //
    // Draw the walk speed

    cv::line(outputMat, cv::Point(WALK_LINE_START, WALK_LINE_HEIGHT), cv::Point(WALK_LINE_END, WALK_LINE_HEIGHT), COLOR_1, WALK_LINE_THICKNESS);
    cv::line(outputMat, cv::Point(LEFT_PART_WIDTH + 30, WALK_LINE_HEIGHT), cv::Point(LEFT_PART_WIDTH + 70, WALK_LINE_HEIGHT), COLOR_3, WALK_LINE_THICKNESS);
    cv::line(outputMat, cv::Point(IMG_WIDTH - 30, WALK_LINE_HEIGHT), cv::Point(IMG_WIDTH - 70, WALK_LINE_HEIGHT), COLOR_3, WALK_LINE_THICKNESS);

    // Draw vertical line and walk speed value
    if(camInfo.user.walkSpeed != -1)
        guiWalkSpeed = camInfo.user.walkSpeed;
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

    std::string textWalkSpeed = "??";
    if(camInfo.user.walkSpeed != -1)
        textWalkSpeed = std::to_string(camInfo.user.walkSpeed);
    drawTextCentered(outputMat, textWalkSpeed, cv::Point(currentWalkSpeedX, WALK_LINE_HEIGHT - 100), FONT_FACE, 2, COLOR_2, 2);

    return outputMat;
}

