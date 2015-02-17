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

#include <opencv2/imgproc/imgproc.hpp>

// Use 10000 in the ratio since we don't want to see after 10 meters
#define DEPTH_IMAGE_RATIO (256.0f / 10000.0f)

cv::Point3f OpenCVUtil::pointToCV(const XnVector3D pt)
{
    return cv::Point3f(pt.X, pt.Y, pt.Z);
}

cv::Point2i OpenCVUtil::pointTo2DCV(const XnVector3D pt)
{
    return cv::Point2i(pt.X, pt.Y);
}

void OpenCVUtil::drawJoint(cv::Mat image, const OpenNIUtil::Joint joint, const cv::Scalar color)
{
    if(OpenNIUtil::isJointAcceptable(joint))
        cv::circle(image, OpenCVUtil::pointTo2DCV(joint.projectivePosition), 6, color, CV_FILLED);
}

cv::Mat OpenCVUtil::drawOpenNIData(OpenNIUtil::CameraInformations camInfo)
{
    cv::Mat outputMat = cv::Mat(480, 640, CV_8UC3, cv::Scalar(0,0,0));

    const XnDepthPixel* depthData = camInfo.depthData;
    uint8_t* pixelPtr = (uint8_t*)outputMat.data;
    for(int r=0 ; r < outputMat.rows ; ++r)
    {
        for(int c=0 ; c < outputMat.cols ; ++c)
        {
            const uint16_t realColor = *depthData * DEPTH_IMAGE_RATIO;
            const uint8_t color = realColor > UINT8_MAX ? UINT8_MAX : realColor;

            pixelPtr[r*outputMat.cols*3 + c*3] = color;
            pixelPtr[r*outputMat.cols*3 + c*3 + 1] = color;
            pixelPtr[r*outputMat.cols*3 + c*3 + 2] = color;

            depthData++;
        }
    }

    // Now draw the joints of the user
    OpenCVUtil::drawJoint(outputMat, camInfo.user.leftLeg.hip, CV_RGB(128, 0, 0));
    OpenCVUtil::drawJoint(outputMat, camInfo.user.leftLeg.knee, CV_RGB(255, 69, 0));
    OpenCVUtil::drawJoint(outputMat, camInfo.user.leftLeg.foot, CV_RGB(218, 165, 32));

    OpenCVUtil::drawJoint(outputMat, camInfo.user.rightLeg.hip, CV_RGB(0, 0, 255));
    OpenCVUtil::drawJoint(outputMat, camInfo.user.rightLeg.knee, CV_RGB(30, 144, 255));
    OpenCVUtil::drawJoint(outputMat, camInfo.user.rightLeg.foot, CV_RGB(127, 255, 212));

    // Print the rotation on screen
    std::string text("Rotation: ");
    text += std::to_string(camInfo.user.rotation);
    cv::putText(outputMat, text, cv::Point(0, outputMat.rows-10), cv::FONT_HERSHEY_SIMPLEX, 1.5, CV_RGB(210, 105, 30), 3);

    return outputMat;
}
