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

#include "controllerchoicewidget.h"
#include "../interfaces/controllercommon.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QString>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDebug>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QButtonGroup>

ControllerChoiceWidget::ControllerChoiceWidget(QWidget *parent) : QWidget(parent)
{
    // Create the main group box
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    _groupBox = new QGroupBox(tr("Choose your favorite controller:"), this);
    mainLayout->addWidget(_groupBox);

    // Add a checkbox for each controller plugin
    _buttonGroup = new QButtonGroup(this);
    QVBoxLayout *groupLayout = new QVBoxLayout();
    _groupBox->setLayout(groupLayout);

    QDir path(QCoreApplication::applicationDirPath());
    if(path.cd(QStringLiteral("controllers")))
    {
        qDebug() << qPrintable(tr("Controllers dir: \"%1\"").arg(path.canonicalPath()));
        QDirIterator iterator(path, QDirIterator::Subdirectories);
        while(iterator.hasNext())
        {
            iterator.next();
            if(!iterator.fileInfo().isDir())
            {
                QFileInfo info(iterator.fileInfo());
                // Check file suffix
                if(info.suffix() == PLUGINS_EXT_WITHOUT_DOT)
                {
                    const QString filePath = info.canonicalFilePath();
                    qDebug() << qPrintable(tr("Find a controller file at \"%1\"").arg(filePath));

                    // Creating the loader to acces the metadata.
                    QPluginLoader *loader = new QPluginLoader(filePath);
                    QJsonObject jsonObj = loader->metaData().value(QStringLiteral("MetaData")).toObject();

                    const QString internalName = jsonObj.value(QStringLiteral("internalName")).toString();
                    const QString name = jsonObj.value(QStringLiteral("name")).toString();
                    const QString description = jsonObj.value(QStringLiteral("description")).toString();

                    if(internalName.isEmpty() || name.isEmpty() || description.isEmpty())
                    {
                        qWarning() << qPrintable(tr("The controller plugin at \"%1\" doesn't specify a name or a description").arg(filePath));
                        delete loader;
                        loader = nullptr;
                        continue;
                    }

                    _controllersMap.insert(internalName, loader);

                    QCheckBox *box = new QCheckBox(QString("%1: %2").arg(name, description), this);
                    box->setProperty("internalName", internalName);
                    _buttonGroup->addButton(box);
                    groupLayout->addWidget(box);
                }
            }
        }
        qDebug() << qPrintable(tr("All controllers plugins are loaded !"));
    }
    else
    {
        qCritical() << qPrintable(tr("Controllers dir doesn't exist at \"%1\"").arg(path.canonicalPath()));
    }
}

void ControllerChoiceWidget::selectController(const QString name)
{
    for(QAbstractButton *button : _buttonGroup->buttons())
    {
        if(button->property("internalName").toString() == name)
        {
            button->setChecked(true);
            return;
        }
    }
    // Here, no plugin with the specified name was found
    qWarning() << qPrintable(tr("The specified controller \"%1\" doesn't exists !"));
}

QString ControllerChoiceWidget::selectedControllerName()
{
    return _buttonGroup->checkedButton()->property("internalName").toString();
}

ControllerInterface *ControllerChoiceWidget::selectedController()
{
    // Find the selected plugin
    QPluginLoader *loader = _controllersMap.value(selectedControllerName());
    // Try to load the plugin
    if(ControllerInterface *plugin = qobject_cast<ControllerInterface *>(loader->instance()))
        return plugin;

    qCritical() << qPrintable(tr("Could not load controller plugin \"%1\" (%2).").arg(selectedControllerName(), loader->errorString()));
    return nullptr;
}

