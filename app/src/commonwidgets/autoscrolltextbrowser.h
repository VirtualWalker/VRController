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

#ifndef AUTOSCROLLTEXTBROWSER_H
#define AUTOSCROLLTEXTBROWSER_H

#include <QTextBrowser>

// Simple class that just scroll the text area to the end on focus lost.
class AutoScrollTextBrowser: public QTextBrowser
{
        Q_OBJECT
    public:
        explicit AutoScrollTextBrowser(QWidget *parent = nullptr);

    public slots:
        // Used to scroll to the down
        void scrollToDown();

    protected:
        void focusOutEvent(QFocusEvent *event);

};

#endif // AUTOSCROLLTEXTBROWSER_H
