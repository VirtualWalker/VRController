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

#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include <QList>
#include "../core/licenses.h"

// Simple class that show informations about this application
class AboutDialog : public QDialog {

    // This prevent the copy of the object
    private:
        Q_OBJECT
        Q_DISABLE_COPY(AboutDialog)

    public:
        AboutDialog(QList<LicenseObject> othersLicenses, QWidget *parent = nullptr);

    private:
        QString licenseToString(LicenseObject license);
};

#endif // ABOUT_H
