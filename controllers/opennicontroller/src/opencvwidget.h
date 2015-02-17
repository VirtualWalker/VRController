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

#ifndef OPENCVWIDGET_H
#define OPENCVWIDGET_H

#include <QGLWidget>
#include <opencv2/core/core.hpp>

// Simple viewer for OpenCV images
class OpenCVWidget: public QGLWidget
{
        Q_OBJECT

    public:
        explicit OpenCVWidget(QWidget *parent = 0);

    public slots:
        // Used to set the image to be viewed
        bool showImage(cv::Mat image);

    protected:
        // OpenGL initialization
        void initializeGL();
        // OpenGL Rendering
        void paintGL();
        // Widget Resize Event
        void resizeGL(int width, int height);

        void updateScene();
        void renderImage();

    private:
        // Indicates when OpenGL view is to be redrawn
        bool _sceneChanged;

        // Qt image to be rendered
        QImage _renderQtImg;
        // Original OpenCV image to be shown
        cv::Mat _origImage;

        // Background color
        QColor _bgColor;

        // Resized Image height
        int _outHeight;
        // Resized Image width
        int _outWidth;
        // height/width ratio
        float _imgRatio;

        // Top left X position to render image in the center of widget
        int _posX;
        // Top left Y position to render image in the center of widget
        int _posY;
};

#endif // OPENCVWIDGET_H
