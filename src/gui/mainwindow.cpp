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
#include "log/logbrowser.h"

#include "about.h"

#include <QDebug>
#include <QCoreApplication>
#include <QTimerEvent>
#include <QCloseEvent>
#include <QVariant>
#include <QDialog>
#include <QApplication>
#include <QMenu>
#include <QAction>

#include <cerrno>
#include <string>

const QString settingChannelStr = "channel";
const QString settingUseCustomChannelStr = "useCustomChannel";
const QString settingFrequencyStr = "frequency";

const QString settingWinGroupStr = "MainWindow";
const QString settingWinSizeStr = "size";
const QString settingWinPosStr = "pos";
const QString settingWinStateStr = "state";

const QString settingLogGroupStr = "Log";
const QString settingLogShowDateStr = "showDate";

MainWindow::MainWindow(LogBrowser *logBrowser)
{
    setWindowTitle(APPLICATION_NAME);

    _logBrowser = logBrowser;
    _settings = new QSettings(this);

    // Init log browser parents
    if(_logBrowser != nullptr)
    {
        _logBrowser->setParent(this);
        _logBrowser->widget()->setParent(this);
    }

    // Set the central widget
    _centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    _centralWidget->setLayout(mainLayout);
    setCentralWidget(_centralWidget);

    _listeningWidget = new ListeningWidget(this);
    mainLayout->addWidget(_listeningWidget);

    // Connect the start button in the listening widget
    connect(_listeningWidget, &ListeningWidget::startListening, this, [this]() {
        if(_listeningWidget->useCustomChannel())
            _btMgr = new BluetoothManager(_listeningWidget->channel(), _btMgrStateHandler, _btMgrErrorHandler);
        else
            _btMgr = new BluetoothManager(AUTO_RFCOMM_CHANNEL, _btMgrStateHandler, _btMgrErrorHandler);

        const int oldChannel = _btMgr->rfcommChannel();

        qDebug() << qPrintable(tr("UUID used for the SDP service: %1").arg(_btMgr->serviceUUID().c_str()));
        qDebug() << qPrintable(tr("Start listening on channel %1.").arg(_btMgr->rfcommChannel()));
        _btMgr->startListening();

        // Re-print the real channel if auto-generated
        if(oldChannel == 0)
            qDebug() << qPrintable(tr("RFCOMM channel has been auto-generated to %1.").arg(_btMgr->rfcommChannel()));
    });

    _fakeController = new FakeController(this);
    _fakeController->hide();
    connect(this, &MainWindow::showFakeController, _fakeController, &FakeController::show);
    connect(this, &MainWindow::setConnectionText, _fakeController, &FakeController::setConnectionAddress);
    mainLayout->addWidget(_fakeController, 1);

    // Add the log browser if needed
    if(_logBrowser != nullptr)
        mainLayout->addWidget(_logBrowser->widget());

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

    // Create the menu bar
    _menuBar = new QMenuBar(this);
    setMenuBar(_menuBar);
    QMenu *aboutMenu = new QMenu(tr("&About"), _menuBar);
    _menuBar->addMenu(aboutMenu);

    QAction *aboutAction = new QAction(tr("&About"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
    aboutMenu->addAction(aboutAction);

    QAction *aboutQtAction = new QAction(tr("About &Qt"), this);
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
    aboutMenu->addAction(aboutQtAction);

    // Connect the signal to start the data timer
    connect(this, &MainWindow::startDataTimer, this, [this]() {
        _btTimer = startTimer(1000/_listeningWidget->frequency(), Qt::PreciseTimer);
    });

    // Create the Bluetooth manager and the handlers
    _btMgrStateHandler = [this](BluetoothManager::State newState) {
        const QString str = BluetoothManager::stateString(newState).c_str();
        emit setStateText(tr("State: %1").arg(str));
        qDebug() << qPrintable(tr("State changed ! New state: %1").arg(str));

        // Check for connected state
        //if(newState == BluetoothManager::State::LISTENING)
        if(newState == BluetoothManager::State::CONNECTED_TO_CLIENT)
        {
            qDebug() << qPrintable(tr("Connected to %1 on channel %2.").arg(_btMgr->clientAddress().c_str()).arg(_btMgr->clientChannel()));
            _listeningWidget->connected();
            _listeningWidget->hide();
            qDebug() << qPrintable(tr("Showing the fake controller ..."));
            emit setConnectionText(_btMgr->clientAddress().c_str(), _btMgr->clientChannel());
            emit showFakeController();

            qDebug() << qPrintable(tr("Start sending data %1 times per second.").arg(_listeningWidget->frequency()));
            qDebug() << qPrintable(tr("Output Bluetooth data in the console every second ..."));

            // Start a timer to send datas at the specified interval
            emit startDataTimer();
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

    // Here, we can read the settings and restore states
    readSettings();
}

// Re-implemented protected method
void MainWindow::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == _btTimer)
    {
        _numberOfTimerExec++;
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

        // Show the debug message only one time per second
        if(_numberOfTimerExec == _listeningWidget->frequency())
        {
            // Don't show debug message if all data equals 0
            if(walkSpeed != 0 || orientation != 0)
                qDebug() << qPrintable(tr("Send message: speed=%1 orientation=%2 (real orientation: %3)").arg((int)msg[1]).arg((int)msg[2]).arg(orientation));
            _numberOfTimerExec = 0;
        }

        _btMgr->sendMessage(&msg, 3);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

// Public slots
void MainWindow::about()
{
    AboutDialog *dialog = new AboutDialog(this);
    dialog->exec();
}

// Protected methods to save and restore the settings
void MainWindow::readSettings()
{
    _listeningWidget->setCustomChannelUse(_settings->value(settingUseCustomChannelStr, false).toBool());
    _listeningWidget->setChannel(_settings->value(settingChannelStr, DEFAULT_RFCOMM_CHANNEL).toInt());
    _listeningWidget->setFrequency(_settings->value(settingFrequencyStr, DEFAULT_MSG_FREQUENCY).toInt());

    _settings->beginGroup(settingWinGroupStr);
    resize(_settings->value(settingWinSizeStr, QSize(600, 0)).toSize());
    move(_settings->value(settingWinPosStr, pos()).toPoint());
    restoreState(_settings->value(settingWinStateStr).toByteArray());
    _settings->endGroup();

    _settings->beginGroup(settingLogGroupStr);
    if(_logBrowser != nullptr)
        _logBrowser->widget()->setShowDate(_settings->value(settingLogShowDateStr, true).toBool());
    _settings->endGroup();
}

void MainWindow::writeSettings()
{
    _settings->setValue(settingUseCustomChannelStr, _listeningWidget->useCustomChannel());
    _settings->setValue(settingChannelStr, _listeningWidget->channel());
    _settings->setValue(settingFrequencyStr, _listeningWidget->frequency());

    _settings->beginGroup(settingWinGroupStr);
    _settings->setValue(settingWinPosStr, pos());
    _settings->setValue(settingWinSizeStr, size());
    _settings->setValue(settingWinStateStr, saveState());
    _settings->endGroup();

    _settings->beginGroup(settingLogGroupStr);
    if(_logBrowser != nullptr)
        _settings->setValue(settingLogShowDateStr, _logBrowser->widget()->showDate());
    _settings->endGroup();
}

