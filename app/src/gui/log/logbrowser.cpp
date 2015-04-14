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

#include "logbrowser.h"

LogBrowser::LogBrowser(QObject *parent): QObject(parent)
{
    _browserWidget = new LogBrowserWidget();
    _browserWidget->setShowDate(true);
    _browserWidget->show();
}

LogBrowser::~LogBrowser()
{
    _browserWidget->deleteLater();
}

LogBrowserWidget* LogBrowser::widget()
{
    return _browserWidget;
}

void LogBrowser::outputMessage(const QString &msg)
{
    QMetaObject::invokeMethod(_browserWidget, "outputMessage", Q_ARG(QString, msg));
}
