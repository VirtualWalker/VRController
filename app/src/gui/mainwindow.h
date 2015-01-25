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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QThread>
#include <QStatusBar>
#include <QMenuBar>
#include <QSettings>

#include "listeningwidget.h"
#include "controllerchoicewidget.h"
#include "../core/bluetoothmanager.h"
#include "log/logbrowser.h"

#define DEFAULT_MSG_FREQUENCY 10

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(LogBrowser* logBrowser);

    public slots:

        void about();
        void setConnectionAddress(const QString addr, const int channel);

    signals:
        //
        // This signals allow the application to manipulate the widgets from an another thread
        // For example, from a bluetooth thread
        void showController();
        void setConnectionText(QString text, int channel);
        void setStateText(QString text);
        void setErrorText(QString text);
        void addStatusBarWidget(QWidget *widget, int stretch = 0);
        void showErrorWidget();

        void startDataTimer();

    protected:
        // Used to send BT data
        void timerEvent(QTimerEvent *event);

        // Used to store the settings
        void closeEvent(QCloseEvent *event);

        void writeSettings();
        void readSettings();

    private:
        QWidget *_centralWidget;

        QVBoxLayout *_mainLayout;

        ControllerChoiceWidget *_controllerChoiceWidget;
        ListeningWidget *_listeningWidget;
        QLabel *_connectionLabel;
        LogBrowser *_logBrowser;

        ControllerInterface *_controllerPlugin;

        QStatusBar *_statusBar;
        // Widgets used in the status bar
        QLabel *_sbState;
        QLabel *_sbError;

        QMenuBar *_menuBar;

        BluetoothManager *_btMgr;
        int _btTimer = 0;
        // Functions to handle states and errors of the BT Manager
        std::function<void(BluetoothManager::State)> _btMgrStateHandler;
        std::function<void(BluetoothManager::Error)> _btMgrErrorHandler;

        // These variables is used to count the number of executions of the method timerEvent()
        int _numberOfTimerExec = 0;

        QSettings *_settings;
};

#endif // MAINWINDOW_H
