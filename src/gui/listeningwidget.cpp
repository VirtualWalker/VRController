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

#include "listeningwidget.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

ListeningWidget::ListeningWidget(QWidget *parent): QWidget(parent)
{
    _strStart = tr("Start listening ...");
    _strListening = tr("Listening ...");

    QVBoxLayout *vboxLayout = new QVBoxLayout(this);
    setLayout(vboxLayout);

    QHBoxLayout *hblStart = new QHBoxLayout();
    vboxLayout->addLayout(hblStart);

    // Set the start button
    _buttonStartListening = new QPushButton(_strStart, this);
    hblStart->addWidget(_buttonStartListening);

    // Disabled the button on click
    connect(_buttonStartListening, &QPushButton::clicked, this, [this]() {
        // You can't start a new listening
        _buttonStartListening->setText(_strListening);
        _listeningProgressIndicator->startAnimation();
        _listeningProgressIndicator->show();
        setEnabled(false);

        // Emit a signal
        emit startListening();

    });

    // Set the listening indicator
    _listeningProgressIndicator = new QProgressIndicator(this);
    hblStart->addWidget(_listeningProgressIndicator);
    _listeningProgressIndicator->hide();

    _channelGroup = new QGroupBox(tr("Use a custom channel"), this);
    _channelGroup->setCheckable(true);
    vboxLayout->addWidget(_channelGroup);

    QFormLayout *groupLayout = new QFormLayout();
    _channelGroup->setLayout(groupLayout);

    _channelBox = new QSpinBox(this);
    _channelBox->setRange(1, 30);
    groupLayout->addRow(tr("Choose the RFCOMM channel:"), _channelBox);

    QFormLayout *formLayout = new QFormLayout();
    vboxLayout->addLayout(formLayout);

    _frequencyBox = new QSpinBox(this);
    _frequencyBox->setRange(1, 100);
    formLayout->addRow(tr("Set the frequency of data send:"), _frequencyBox);
}

int ListeningWidget::channel() const
{
    return _channelBox->value();
}

int ListeningWidget::frequency() const
{
    return _frequencyBox->value();
}

bool ListeningWidget::useCustomChannel() const
{
    return _channelGroup->isChecked();
}

// Public slot
void ListeningWidget::connected()
{
    setEnabled(true);
    _buttonStartListening->setText(_strStart);
    _listeningProgressIndicator->stopAnimation();
    _listeningProgressIndicator->hide();
}

void ListeningWidget::setChannel(int value)
{
    if(value > 0 && value <= 30)
        _channelBox->setValue(value);
}

void ListeningWidget::setFrequency(int value)
{
    if(value > 0 && value <= 100)
        _frequencyBox->setValue(value);
}

void ListeningWidget::setCustomChannelUse(bool use)
{
    _channelGroup->setChecked(use);
}

