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

#include <QKeyEvent>

FakeController::FakeController(QWidget *parent): QWidget(parent)
{
    _mainLayout = new QGridLayout(this);
    setLayout(_mainLayout);

    setFocusPolicy(Qt::StrongFocus);
    grabKeyboard();

    _connectionLabel = new QLabel(this);
    _connectionLabel->setAlignment(Qt::AlignCenter);
    _mainLayout->addWidget(_connectionLabel, 0, 0, 1, 2);

    //
    // Set the orientation part
    //

    _orientationLayout = new QVBoxLayout();

    _orientationDial = new QDial(this);
    _orientationDial->setNotchesVisible(true);
    _orientationDial->setRange(0, MAX_ORIENTATION);
    _orientationDial->setWrapping(true);
    _orientationLayout->addWidget(_orientationDial, 1);

    _orientationValue = new QSpinBox(this);
    _orientationValue->setRange(0, MAX_ORIENTATION);
    _orientationValue->setSuffix(tr(" degrees"));
    _orientationValue->setWrapping(true);
    _orientationLayout->addWidget(_orientationValue);

    _orientationValueLabel = new QLabel(this);
    _orientationValueLabel->setAlignment(Qt::AlignCenter);
    updateOrientationLabel(_orientationDial->value());
    connect(_orientationDial, &QDial::valueChanged, this, &FakeController::updateOrientationLabel);
    connect(_orientationValue, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &FakeController::updateOrientationLabel);
    _orientationLayout->addWidget(_orientationValueLabel);

    _orientationLabel = new QLabel(this);
    _orientationLabel->setAlignment(Qt::AlignCenter);
    _orientationLabel->setText(tr("Set the orientation of the player."));
    _orientationLayout->addWidget(_orientationLabel);

    _orientationBox = new QGroupBox(this);
    _orientationBox->setLayout(_orientationLayout);
    _mainLayout->addWidget(_orientationBox, 1, 0, 1, 1);

    //
    // Set the walk speed part
    //

    _walkSpeedLayout = new QVBoxLayout();

    _walkSpeedDial = new QDial(this);
    _walkSpeedDial->setNotchesVisible(true);
    _walkSpeedDial->setRange(0, MAX_WALK_SPEED);
    _walkSpeedLayout->addWidget(_walkSpeedDial, 1);

    _walkSpeedValue = new QSpinBox(this);
    _walkSpeedValue->setRange(0, MAX_WALK_SPEED);
    _walkSpeedLayout->addWidget(_walkSpeedValue);

    _walkSpeedValueLabel = new QLabel(this);
    _walkSpeedValueLabel->setAlignment(Qt::AlignCenter);
    updateWalkSpeedLabel(_walkSpeedDial->value());
    connect(_walkSpeedDial, &QDial::valueChanged, this, &FakeController::updateWalkSpeedLabel);
    connect(_walkSpeedValue, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &FakeController::updateWalkSpeedLabel);
    _walkSpeedLayout->addWidget(_walkSpeedValueLabel);

    _walkSpeedLabel = new QLabel(this);
    _walkSpeedLabel->setAlignment(Qt::AlignCenter);
    _walkSpeedLabel->setText(tr("Set the walk speed of the player."));
    _walkSpeedLayout->addWidget(_walkSpeedLabel);

    _walkSpeedBox = new QGroupBox(this);
    _walkSpeedBox->setLayout(_walkSpeedLayout);
    _mainLayout->addWidget(_walkSpeedBox, 1, 1, 1, 1);
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

// Public slots
void FakeController::setConnectionAddress(const QString addr, const int channel)
{
    _connectionLabel->setText("<b>" + tr("Connected to device %1 on channel %2").arg(addr).arg(channel) + "</b>");
}

// Private slots
void FakeController::updateOrientationLabel(int newValue)
{
    _orientationValueLabel->setText("<b>" + tr("%1 degrees").arg(newValue) + "</b>");

    _orientationDial->setValue(newValue);
    _orientationValue->setValue(newValue);

    emit orientationChanged(newValue);
    emit valueChanged();
}

void FakeController::updateWalkSpeedLabel(int newValue)
{
    _walkSpeedValueLabel->setText("<b>" + QString::number(newValue) + "</b>");

    _walkSpeedDial->setValue(newValue);
    _walkSpeedValue->setValue(newValue);

    emit walkSpeedChanged(newValue);
    emit valueChanged();
}

// Re-implemented protected methods
void FakeController::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Plus:
            _walkSpeedDial->setValue(_walkSpeedDial->value() + 1);
            break;
        case Qt::Key_NumberSign:
        case Qt::Key_Minus:
            _walkSpeedDial->setValue(_walkSpeedDial->value() - 1);
            break;
        default:
            break;
    }

    event->accept();
}

