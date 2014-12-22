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

#ifndef LOGBROWSER_H
#define LOGBROWSER_H

#include <QObject>

#include "logbrowserwidget.h"

class LogBrowser : public QObject
{
        Q_OBJECT

    public:
        explicit LogBrowser(QObject *parent = nullptr);
        ~LogBrowser();

        LogBrowserWidget* widget();

    public slots:
        void outputMessage(const QString &msg);

    signals:
        void sendMessage(const QString &msg);

    private:
        LogBrowserWidget *_browserWidget;

};

#endif // LOGBROWSER_H
