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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QThread>
#include <QStatusBar>
#include <QTimerEvent>

#include "listeningwidget.h"
#include "fakecontroller.h"

#include "../core/bluetoothmanager.h"

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);

    signals:
        //
        // This signals allow the application to manipulate the widgets from an another thread
        // For example, from a bluetooth thread
        void showFakeController();
        void setStateText(QString text);
        void setErrorText(QString text);
        void addStatusBarWidget(QWidget *widget, int stretch = 0);
        void showErrorWidget();

    protected:
        // Used to send BT data
        void timerEvent(QTimerEvent *event);

    private:
        QWidget *_centralWidget;

        ListeningWidget *_listeningWidget;
        FakeController *_fakeController;

        QStatusBar *_statusBar;
        // Widgets used in the status bar
        QLabel *_sbState;
        QLabel *_sbError;

        BluetoothManager *_btMgr;
        int _btTimer;
        // Functions to handle states and errors of the BT Manager
        std::function<void(BluetoothManager::State)> _btMgrStateHandler;
        std::function<void(BluetoothManager::Error)> _btMgrErrorHandler;
};

#endif // MAINWINDOW_H
