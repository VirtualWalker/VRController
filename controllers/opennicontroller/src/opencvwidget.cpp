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

#include "opencvwidget.h"

OpenCVWidget::OpenCVWidget(QWidget *parent): QGLWidget(parent)
{
    _sceneChanged = false;
    _bgColor = QColor::fromRgb(150, 150, 150);

    _outHeight = 0;
    _outWidth = 0;
    _imgRatio = 4.0f/3.0f;

    _posX = 0;
    _posY = 0;
}

void OpenCVWidget::initializeGL()
{
    makeCurrent();
    qglClearColor(_bgColor.darker());
}

void OpenCVWidget::resizeGL(int width, int height)
{
    makeCurrent();
    qglClearColor(_bgColor.darker());
    glViewport(0, 0, (GLint)width, (GLint)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Draw image in the center of the area
    glOrtho(0, width, 0, height, 0, 1);

    glMatrixMode(GL_MODELVIEW);

    // Scale image if needed
    _outHeight = width / _imgRatio;
    _outWidth = width;

    if(_outHeight > height)
    {
        _outWidth = height * _imgRatio;
        _outHeight = height;
    }

    _posX = (width - _outWidth) / 2;
    _posY = (height - _outHeight) / 2;

    _sceneChanged = true;

    updateScene();
}

void OpenCVWidget::updateScene()
{
    if(_sceneChanged && isVisible())
        updateGL();
}

void OpenCVWidget::paintGL()
{
    makeCurrent();

    if(!_sceneChanged)
        return;

    renderImage();
    _sceneChanged = false;
}

void OpenCVWidget::renderImage()
{
    makeCurrent();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(!_renderQtImg.isNull())
    {
        glLoadIdentity();
        glPushMatrix();

        QImage image;

        const int imWidth = _renderQtImg.width();
        const int imHeight = _renderQtImg.height();

        // The image need to be resized to fit the widget ?
        if(imWidth != size().width() && imHeight != size().height())
            image = _renderQtImg.scaled(QSize(_outWidth, _outHeight), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        else
            image = _renderQtImg;

        // Centering image in draw area
        glRasterPos2i(_posX, _posY);

        glDrawPixels(image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
        glPopMatrix();
        glFlush();
    }
}

bool OpenCVWidget::showImage(cv::Mat image)
{
    image.copyTo(_origImage);

    _imgRatio = (float)image.cols/(float)image.rows;

    if(_origImage.channels() == 3)
        _renderQtImg = QImage((const unsigned char*)(_origImage.data),
                              _origImage.cols, _origImage.rows,
                              _origImage.step, QImage::Format_RGB888).rgbSwapped();
    else if(_origImage.channels() == 1)
        _renderQtImg = QImage((const unsigned char*)(_origImage.data),
                              _origImage.cols, _origImage.rows,
                              _origImage.step, QImage::Format_Indexed8);
    else
        return false;

    _renderQtImg = QGLWidget::convertToGLFormat(_renderQtImg);
    _sceneChanged = true;
    updateScene();

    return true;
}
