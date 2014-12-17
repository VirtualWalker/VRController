/*
 * This file is part of VRController.
 * Copyright (c) 2014 Fabien Caylus <toutjuste13@gmail.com>
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

#ifndef FAKECONTROLLER_H
#define FAKECONTROLLER_H

#include <QLabel>
#include <QSpinBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QEvent>

#include "dial.h"

#define MAX_WALK_SPEED 254
#define MAX_ORIENTATION 359

// This class allow the user to control some params like the walk speed and the orientation
class FakeController : public QWidget
{
        Q_OBJECT
    public:
        explicit FakeController(QWidget *parent = 0);

        // Getters
        int orientationValue() const;
        int walkSpeedValue() const;

    public slots:

        void setConnectionAddress(const QString addr, const int channel);

    signals:
        void orientationChanged(int newOrientation);
        void walkSpeedChanged(int newWalkSpeed);

        // Emitted when the orientation or the walk speed changed.
        void valueChanged();

    private:
        QGridLayout *_mainLayout;

        // Dials to control the params
        Dial *_orientationDial; // A number between 1 and 360 degrees
        Dial *_walkSpeedDial; // A speed between 0 and 254 (since 255 means the beginning of a transmission)

        QVBoxLayout *_orientationLayout;
        QVBoxLayout *_walkSpeedLayout;

        QLabel *_orientationLabel;
        QLabel *_walkSpeedLabel;

        QLabel *_orientationValueLabel;
        QLabel *_walkSpeedValueLabel;

        QSpinBox *_orientationValue;
        QSpinBox *_walkSpeedValue;

        QGroupBox *_orientationBox;
        QGroupBox *_walkSpeedBox;

        QLabel *_connectionLabel;

    protected:
        void keyPressEvent(QKeyEvent *event);

    private slots:

        void updateOrientationLabel(int newValue);
        void updateWalkSpeedLabel(int newValue);
};

#endif // FAKECONTROLLER_H
