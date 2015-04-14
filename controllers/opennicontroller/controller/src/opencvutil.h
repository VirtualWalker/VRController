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

#ifndef OPENCVUTIL_H
#define OPENCVUTIL_H

#include <string>
#include <opencv2/core/core.hpp>
#include "../../vropenni-catcher/src/openniutil.h"

// Drawing functions
namespace OpenCVUtil
{
    cv::Point3f pointToCV(const XnVector3D pt);
    cv::Point2i pointTo2DCV(const XnVector3D pt, const int offsetX = 0, const int offsetY = 0, const int res = 1);

    // Draw a joint
    void drawJoint(cv::Mat& image, const OpenNIUtil::Joint joint, const cv::Scalar color,
                   const int offsetX = 0, const int offsetY = 0, const int res = 1);
    // Draw a lamb between two joints
    void drawLimb(cv::Mat& image, const OpenNIUtil::Joint joint1, const OpenNIUtil::Joint joint2, const cv::Scalar color,
                  const int offsetX = 0, const int offsetY = 0, const int res = 1);

    void drawJointsOfUser(cv::Mat& image, const OpenNIUtil::User user, const cv::Scalar rightColor, const cv::Scalar leftColor,
                          const int offsetX = 0, const int offsetY = 0, const int res = 1);
    void drawLimbsOfUsers(cv::Mat& image, const OpenNIUtil::User user, const cv::Scalar color,
                          const int offsetX = 0, const int offsetY = 0, const int res = 1);

    // Draw a text centered relative to the specified point
    void drawTextCentered(cv::Mat& image, const std::string& text, const cv::Point& centerPoint,
                          const int &fontFace, const double &fontScale, const cv::Scalar& color,
                          const int &thickness);

    void drawDepthMap(cv::Mat& image, XnDepthPixel *depthMap,
                      const int startX, const int startY, const int res = 1);

    // Draw all informations and return the image
    cv::Mat drawOpenNIData(OpenNIUtil::CameraInformations camInfo, OpenNIUtil::DepthMaps depthMaps);
}

#endif // OPENCVUTIL_H
