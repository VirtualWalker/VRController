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

#ifndef LOGBROWSERWIDGET_H
#define LOGBROWSERWIDGET_H

#include <QPushButton>
#include <QCheckBox>

#include "../../commonwidgets/autoscrolltextbrowser.h"

class LogBrowserWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool showDate READ showDate WRITE setShowDate NOTIFY showDateChanged)

    public:
        LogBrowserWidget(QWidget *parent = nullptr);

        bool showDate() const;

    public slots:
        void outputMessage(const QString &msg);
        void setShowDate(bool shown);

        void scrollToDown();

    signals:
        void showDateChanged(bool shown);

    private slots:
        void save();
        void checkBoxChanged();

    protected:
        AutoScrollTextBrowser *_browser;
        QPushButton *_clearButton;
        QPushButton *_saveButton;

        QCheckBox *_checkBoxDate;

        bool _showDate;
};

#endif // LOGBROWSERWIDGET_H
