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

#include <QWidget>

#include <QDial>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QEvent>

// This class allow the user to control some params like the walk speed and the orientation
class FakeController : public QWidget
{
        Q_OBJECT
    public:
        explicit FakeController(QWidget *parent = 0);

        // Getters
        int orientationValue() const;
        int walkSpeedValue() const;

    signals:
        void orientationChanged(int newOrientation);
        void walkSpeedChanged(int newWalkSpeed);

        // Emitted when the orientation or the walk speed changed.
        void valueChanged();

    private:
        QGridLayout *_mainLayout;

        // Dials to control the params
        QDial *_orientationDial; // A number between 1 and 360 degrees
        QDial *_walkSpeedDial; // A speed between 0 and 254 (since 255 means the beginning of a transmission)

        QVBoxLayout *_orientationLayout;
        QVBoxLayout *_walkSpeedLayout;

        QLabel *_orientationLabel;
        QLabel *_walkSpeedLabel;

        QLabel *_orientationValue;
        QLabel *_walkSpeedValue;

    private slots:

        void updateOrientationLabel(int newValue);
        void updateWalkSpeedLabel(int newValue);
};

#endif // FAKECONTROLLER_H
