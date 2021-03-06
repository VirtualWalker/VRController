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

#ifndef LISTENINGWIDGET_H
#define LISTENINGWIDGET_H

#include <QPushButton>
#include <QSpinBox>
#include <QGroupBox>

#include "QProgressIndicator"

class ListeningWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit ListeningWidget(QWidget *parent = nullptr);

        int channel() const;
        int frequency() const;

        bool useCustomChannel() const;

        // Simple function that click on the start button
        void clickOnStartListening();

    signals:
        void startListening();

    public slots:

        // This slot must be call when the BT manager is connected to a client
        void connected();

        // Set the value of the channel box
        void setChannel(int value);

        // Set the frequency for sending data
        void setFrequency(int value);

        // Set if we use a custom channel
        void setCustomChannelUse(bool use);

    private:

        QPushButton *_buttonStartListening;
        QProgressIndicator *_listeningProgressIndicator;

        QString _strStart;
        QString _strListening;

        QSpinBox *_channelBox;
        QSpinBox *_frequencyBox;

        // Used to know if the channel is set manually
        QGroupBox *_channelGroup;
};

#endif // LISTENINGWIDGET_H
