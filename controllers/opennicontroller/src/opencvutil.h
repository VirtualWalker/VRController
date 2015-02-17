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

#include <opencv2/core/core.hpp>
#include "openniutil.h"

// Drawing functions
namespace OpenCVUtil
{
    cv::Point3f pointToCV(const XnVector3D pt);
    cv::Point2i pointTo2DCV(const XnVector3D pt);
    void drawJoint(cv::Mat image, const OpenNIUtil::Joint joint, const cv::Scalar color);
    cv::Mat drawOpenNIData(OpenNIUtil::CameraInformations camInfo);
}

#endif // OPENCVUTIL_H
