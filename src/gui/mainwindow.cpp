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

#include "mainwindow.h"

#include <QDebug>
#include <QCoreApplication>

#include <cerrno>
#include <string>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
    setWindowTitle(APPLICATION_NAME);

    _btTimer = -1;

    // Set the central widget
    _centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    _centralWidget->setLayout(mainLayout);
    setCentralWidget(_centralWidget);

    _listeningWidget = new ListeningWidget(this);
    _listeningWidget->setChannelValue(DEFAULT_RFCOMM_CHANNEL);
    mainLayout->addWidget(_listeningWidget);
    // Connect the start button in the listening widget
    connect(_listeningWidget, &ListeningWidget::startListening, this, [this]() {
        _btMgr = new BluetoothManager(_listeningWidget->channelValue(), _btMgrStateHandler, _btMgrErrorHandler);
        qDebug() << qPrintable(tr("Start listening on channel %1").arg(_btMgr->rfcommChannel()));
        _btMgr->startListening();
    });

    _fakeController = new FakeController(this);
    _fakeController->hide();
    connect(this, &MainWindow::showFakeController, _fakeController, &FakeController::show);
    mainLayout->addWidget(_fakeController);

    // Set the status bar
    _statusBar = new QStatusBar(this);
    connect(this, &MainWindow::addStatusBarWidget, _statusBar, &QStatusBar::addWidget);

    _sbState = new QLabel(_statusBar);
    _sbState->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    connect(this, &MainWindow::setStateText, _sbState, &QLabel::setText);

    _sbError = new QLabel(this);
    _sbError->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    _sbError->hide();
    connect(this, &MainWindow::setErrorText, _sbError, &QLabel::setText);
    connect(this, &MainWindow::showErrorWidget, _sbError, &QLabel::show);

    _statusBar->addPermanentWidget(_sbState);
    setStatusBar(_statusBar);

    // Create the Bluetooth manager and the handlers
    _btMgrStateHandler = [this](BluetoothManager::State newState) {
        const QString str = BluetoothManager::stateString(newState).c_str();
        //_sbState->setText(tr("State: %1").arg(str));
        emit setStateText(tr("State: %1").arg(str));
        qDebug() << qPrintable(tr("State changed ! New state: %1").arg(str));

        // Check for connected state
        if(newState == BluetoothManager::State::CONNECTED_TO_CLIENT)
        {
            qDebug() << qPrintable(tr("Connected to %1 on channel %2.").arg(_btMgr->clientAddress().c_str()).arg(_btMgr->clientChannel()));
            _listeningWidget->connected();
            _listeningWidget->hide();
            qDebug() << qPrintable(tr("Showing the fake controller ..."));
            emit showFakeController();

            // Start a timer to send datas 5 time by second
            _btTimer = startTimer(1000/5, Qt::PreciseTimer);
        }
    };
    _btMgrErrorHandler = [this](BluetoothManager::Error newError) {
        // We show the error only if different to NO_ERROR
        if(newError != BluetoothManager::Error::NO_ERROR)
        {
            const QString str = BluetoothManager::errorString(newError).c_str();
            // Show in red
            emit setErrorText("<font color=\"#ff0000\">" + tr("Error: %1").arg(str) + "</font>");
            emit addStatusBarWidget(_sbError);
            emit showErrorWidget();
            qCritical() << qPrintable(tr("Bluetooth error: %1 (error code: %2)").arg(str).arg(static_cast<int>(newError)));
            // Check for errno description
            if(errno != 0)
                qCritical() << qPrintable(tr("Error detail: %1 (errno value: %2)").arg(strerror(errno)).arg(errno));
        }
    };

    // Call the state handler just one time at the start
    _btMgrStateHandler(BluetoothManager::State::NO_STATE);
}

// Re-implemented protected method
void MainWindow::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == _btTimer)
    {
        if(_btMgr == nullptr)
        {
            qCritical() << qPrintable(tr("The bluetooth manager is not created !"));
            return;
        }

        const int walkSpeed = _fakeController->walkSpeedValue();
        const int orientation = _fakeController->orientationValue();
        // The message contains 3 numbers
        // First: 0xFF --> specify that the message begins
        // Second: Walk speed, a number between 0 and 254
        // Third: Orientation, a number between 0 and 254 (a ratio with the originally 0-360 range)
        std::uint8_t msg[3];

        msg[0] = 0xFF;
        msg[1] = walkSpeed;
        msg[2] = orientation / (360.0f/255.0f);
        qDebug() << qPrintable(tr("Send message: speed=%1 orientation=%2 (real orientation: %3)").arg((int)msg[1]).arg((int)msg[2]).arg(orientation));

        _btMgr->sendMessage(&msg, 3);
    }
}

