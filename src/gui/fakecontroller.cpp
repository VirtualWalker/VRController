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

#include "fakecontroller.h"

FakeController::FakeController(QWidget *parent): QWidget(parent)
{
    _mainLayout = new QGridLayout(this);
    setLayout(_mainLayout);

    //
    // Set the orientation part
    //

    _orientationLayout = new QVBoxLayout();

    _orientationDial = new QDial(this);
    _orientationDial->setNotchesVisible(true);
    _orientationDial->setRange(0, 359);
    _orientationDial->setWrapping(true);
    _orientationLayout->addWidget(_orientationDial, 1);

    _orientationValue = new QLabel(this);
    _orientationValue->setAlignment(Qt::AlignCenter);
    updateOrientationLabel(_orientationDial->value());
    connect(_orientationDial, &QDial::valueChanged, this, &FakeController::updateOrientationLabel);
    _orientationLayout->addWidget(_orientationValue);

    _orientationLabel = new QLabel(this);
    _orientationLabel->setAlignment(Qt::AlignCenter);
    _orientationLabel->setText(tr("Set the orientation of the player."));
    _orientationLayout->addWidget(_orientationLabel);

    _mainLayout->addLayout(_orientationLayout, 0, 0, 1, 1);

    //
    // Set the walk speed part
    //

    _walkSpeedLayout = new QVBoxLayout();

    _walkSpeedDial = new QDial(this);
    _walkSpeedDial->setNotchesVisible(true);
    _walkSpeedDial->setRange(0, 254);
    _walkSpeedLayout->addWidget(_walkSpeedDial, 1);

    _walkSpeedValue = new QLabel(this);
    _walkSpeedValue->setAlignment(Qt::AlignCenter);
    updateWalkSpeedLabel(_walkSpeedDial->value());
    connect(_walkSpeedDial, &QDial::valueChanged, this, &FakeController::updateWalkSpeedLabel);
    _walkSpeedLayout->addWidget(_walkSpeedValue);

    _walkSpeedLabel = new QLabel(this);
    _walkSpeedLabel->setAlignment(Qt::AlignCenter);
    _walkSpeedLabel->setText(tr("Set the walk speed of the player."));
    _walkSpeedLayout->addWidget(_walkSpeedLabel);

    _mainLayout->addLayout(_walkSpeedLayout, 0, 1, 1, 1);
}

// Getters
int FakeController::orientationValue() const
{
    return _orientationDial->value();
}

int FakeController::walkSpeedValue() const
{
    return _walkSpeedDial->value();
}

// Private slots
void FakeController::updateOrientationLabel(int newValue)
{
    _orientationValue->setText("<b>" + tr("%1 degrees").arg(newValue) + "</b>");
    emit orientationChanged(newValue);
    emit valueChanged();
}

void FakeController::updateWalkSpeedLabel(int newValue)
{
    _walkSpeedValue->setText("<b>" + QString::number(newValue) + "</b>");
    emit walkSpeedChanged(newValue);
    emit valueChanged();
}

