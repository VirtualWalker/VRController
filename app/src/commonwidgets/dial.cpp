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

#include "dial.h"

#include <QPaintEvent>
#include <QStylePainter>
#include <QStyleOptionSlider>

Dial::Dial(QWidget *parent) : QDial(parent)
{
}

// Getter
bool Dial::isInverted() const
{
    return _inverted;
}

// Public slots
void Dial::invert(bool inverted)
{
    _inverted = inverted;
    emit inversionChanged(_inverted);
}

// Re-implemented
void Dial::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStylePainter painter(this);
    QStyleOptionSlider option;
    initStyleOption(&option);
    if(_inverted)
    {
        painter.rotate(180);
        painter.translate(-width(), -height());
    }
    painter.drawComplexControl(QStyle::CC_Dial, option);
}

void Dial::mousePressEvent(QMouseEvent * e)
{
    QDial::mousePressEvent(e);
    if(_inverted)
        setSliderPosition(maximum()/2 + sliderPosition());
}

void Dial::mouseReleaseEvent(QMouseEvent * e)
{
    QDial::mouseReleaseEvent(e);
    if(_inverted)
        setSliderPosition(maximum()/2 + sliderPosition());
}

void Dial::mouseMoveEvent(QMouseEvent * e)
{
    QDial::mouseMoveEvent(e);
    if(_inverted)
        setSliderPosition(maximum()/2 + sliderPosition());
}

void Dial::keyPressEvent(QKeyEvent * e)
{
    switch (e->key())
    {
        case Qt::Key_Plus:
            setValue(value() + singleStep());
            break;
        case Qt::Key_Minus:
            setValue(value() - singleStep());
            break;
        case Qt::Key_PageUp:
            setValue(value() + pageStep());
            break;
        case Qt::Key_PageDown:
            setValue(value() - pageStep());
            break;
        case Qt::Key_Home:
            setValue(minimum());
            break;
        case Qt::Key_End:
            setValue(maximum());
            break;
        default:
            break;
    }
    e->accept();
}

