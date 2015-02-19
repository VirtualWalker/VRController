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

#ifndef CONTROLLERCHOICEWIDGET_H
#define CONTROLLERCHOICEWIDGET_H

#include <QGroupBox>
#include <QMap>
#include <QPluginLoader>
#include <QButtonGroup>
#include <QList>

#include "../interfaces/controllerinterface.h"
#include "../core/licenses.h"

class ControllerChoiceWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit ControllerChoiceWidget(QWidget *parent = 0);
        ~ControllerChoiceWidget();

        QString selectedControllerName();
        ControllerInterface *selectedController();

        QList<LicenseObject> thirdPartiesLicensesFromPlugins() const;

    public slots:
        void selectController(const QString name);

    private:

        QGroupBox *_groupBox;
        QButtonGroup *_buttonGroup;
        QMap<QString, QPluginLoader *> _controllersMap;

        // This list is filled when parsing the JSON files
        QList<LicenseObject> _thirdPartiesLicenses;
};

#endif // CONTROLLERCHOICEWIDGET_H
