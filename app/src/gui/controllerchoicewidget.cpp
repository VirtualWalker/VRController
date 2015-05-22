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

#include "controllerchoicewidget.h"
#include "../interfaces/controllercommon.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QString>
#include <QJsonArray>
#include <QCoreApplication>
#include <QDebug>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLocale>
#include <QRadioButton>
#include <QPushButton>
#include <QList>
#include <QByteArray>
#include <QImage>

// Some strings used in the JSON parser
// Declaration here should be faster
const QString nameStr = "name";
const QString descStr = "description";
const QString thirdLicenceStr = "3rdLicenses";
const QString i18nStr = "i18n";

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
                    QPluginLoader *loader = new QPluginLoader(filePath, this);
                    QJsonObject jsonObj = loader->metaData().value(QStringLiteral("MetaData")).toObject();

                    const QString internalName = jsonObj.value(QStringLiteral("internalName")).toString();
                    QString name = jsonObj.value(nameStr).toString();
                    QString description = jsonObj.value(descStr).toString();

                    if(internalName.isEmpty() || name.isEmpty() || description.isEmpty())
                    {
                        qWarning() << qPrintable(tr("The controller plugin at \"%1\" doesn't specify a name or a description").arg(filePath));
                        delete loader;
                        loader = nullptr;
                        continue;
                    }

                    // Check for translations
                    QJsonObject currentLocaleObj;
                    if(translationObject(jsonObj, &currentLocaleObj))
                    {
                        // Here there is a translation in the current language, check for "name" and "description"
                        if(currentLocaleObj.contains(nameStr))
                            name = currentLocaleObj.value(nameStr).toString(name);
                        if(currentLocaleObj.contains(descStr))
                            description = currentLocaleObj.value(descStr).toString(description);
                    }

                    // Check for 3rdLicenses
                    if(jsonObj.contains(thirdLicenceStr))
                    {
                        QJsonArray licensesArray = jsonObj.value(thirdLicenceStr).toArray();
                        for(int i=0, end=licensesArray.size(); i<end ; ++i)
                        {
                            QJsonObject licenseObj = licensesArray.at(i).toObject();
                            const QString licenseName = licenseObj.value(nameStr).toString();
                            const QString licenseUrl = licenseObj.value(QStringLiteral("url")).toString();
                            const QString licenseText = licenseObj.value(QStringLiteral("text")).toString();

                            if(licenseName.isEmpty() || licenseText.isEmpty())
                            {
                                qWarning() << qPrintable(tr("Find a third party license for controller \"%1\" but without a name or a correct text. Skip it !").arg(name));
                                continue;
                            }

                            // Add the license to the list
                            _thirdPartiesLicenses.append(LicenseObject(licenseName, licenseUrl, licenseText, name));
                        }
                    }

                    _controllersMap.insert(internalName, loader);

                    QRadioButton *button = new QRadioButton(QString("%1: %2").arg(name, description), this);
                    button->setProperty("internalName", internalName);
                    _buttonGroup->addButton(button);
                    groupLayout->addWidget(button);

                    // Toggle the button one time (refresh the GUI)
                    button->toggle();
                }
            }
        }
        qDebug() << qPrintable(tr("All controllers plugins are loaded !"));
    }
    else
    {
        qCritical() << qPrintable(tr("Controllers dir doesn't exist at \"%1\"").arg(path.canonicalPath()));
    }

    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
}

ControllerChoiceWidget::~ControllerChoiceWidget()
{
    // Unload all plugins
    while(!_controllersMap.isEmpty())
        _controllersMap.take(_controllersMap.firstKey())->unload();
}

// Public slots

void ControllerChoiceWidget::selectController(const QString &name)
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
    qWarning() << qPrintable(tr("The specified controller \"%1\" doesn't exists !").arg(name));
}

// Getters

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

QMap<QString, QPluginLoader *> ControllerChoiceWidget::allControllers()
{
    return _controllersMap;
}

QList<LicenseObject> ControllerChoiceWidget::thirdPartiesLicensesFromPlugins() const
{
    return _thirdPartiesLicenses;
}

// Private
bool ControllerChoiceWidget::translationObject(const QJsonObject& baseObject, QJsonObject *i18nObject)
{
    if(i18nObject == nullptr)
        return false;

    // Check for translations in the current language
    if(baseObject.contains(i18nStr))
    {
        // Get the current locale
        const QString locale = QLocale::system().name().section('_', 0, 0);
        QJsonObject trObj = baseObject.value(i18nStr).toObject();
        if(trObj.contains(locale))
        {
            // Set the object to the current locale object
            *i18nObject = trObj.value(locale).toObject();
            return true;
        }
    }

    return false;
}

