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

// Some strings used in the JSON parser
// Declaration here should be faster
const QString nameStr = "name";
const QString descStr = "description";
const QString internalNameStr = "internalName";
const QString thirdLicenceStr = "3rdLicenses";
const QString i18nStr = "i18n";
const QString launchOptionsStr = "launchOptions";

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

                    const QString internalName = jsonObj.value(internalNameStr).toString();
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

                    // Check for launch options
                    ControllerOptionsList optionsList;
                    if(jsonObj.contains(launchOptionsStr))
                    {
                        QJsonArray optionsArray = jsonObj.value(launchOptionsStr).toArray();
                        for(int i=0, end = optionsArray.size(); i<end; ++i)
                        {
                            QJsonObject optionObject = optionsArray.at(i).toObject();
                            const QString optName = optionObject.value(internalNameStr).toString();
                            QString optDesc = optionObject.value(descStr).toString();

                            if(optName.isEmpty() || optDesc.isEmpty())
                            {
                                qWarning() << qPrintable(tr("Find a launch option for the controller \"%1\" but without a name or a description. Skip it !").arg(name));
                                continue;
                            }

                            // Check for translations
                            QJsonObject optLocaleObj;
                            if(translationObject(optionObject, &optLocaleObj))
                            {
                                if(optLocaleObj.contains(descStr))
                                    optDesc = optLocaleObj.value(descStr).toString(optDesc);
                            }

                            // Now add the option to the list
                            optionsList.insert(optName, QPair<QString, bool>(optDesc, false));
                        }
                    }

                    ControllerWrapper controllerWrapper;
                    controllerWrapper.options = optionsList;
                    controllerWrapper.loader = loader;
                    _controllersMap.insert(internalName, controllerWrapper);

                    QRadioButton *button = new QRadioButton(QString("%1: %2").arg(name, description), this);
                    button->setProperty(internalNameStr.toStdString().c_str(), internalName);
                    _buttonGroup->addButton(button);
                    groupLayout->addWidget(button);

                    QMap<QString, QButtonGroup*> optionsMap;
                    // Add options if needed
                    for(ControllerOptionsList::iterator it = optionsList.begin(), end = optionsList.end() ; it != end ; ++it)
                    {
                        QHBoxLayout *optionsLayout = new QHBoxLayout();
                        QButtonGroup *optButtonGroup = new QButtonGroup(this);
                        optButtonGroup->setProperty("optName", it.key());
                        QRadioButton *yesButton = new QRadioButton(tr("Yes"), this);
                        QRadioButton *noButton = new QRadioButton(tr("No"), this);
                        optButtonGroup->addButton(yesButton, BUTTON_YES_ID);
                        optionsLayout->addWidget(yesButton);
                        optButtonGroup->addButton(noButton, BUTTON_NO_ID);
                        optionsLayout->addWidget(noButton);

                        // Align both buttons to the left side
                        optionsLayout->addStretch(1);

                        QFormLayout *formLayout = new QFormLayout();

                        // Add a margin to indent the content
                        QMargins formLayoutMargin = formLayout->contentsMargins();
                        formLayoutMargin.setLeft(30);
                        formLayout->setContentsMargins(formLayoutMargin);

                        formLayout->addRow(it.value().first, optionsLayout);

                        // Create a main widget
                        // Used to hide all the content
                        QWidget *optMainWidget = new QWidget(this);
                        optMainWidget->setLayout(formLayout);
                        groupLayout->addWidget(optMainWidget);

                        // Connect the widget to the QCheckBox signals
                        connect(button, &QRadioButton::toggled, this, [optMainWidget](bool checked) {
                            if(checked)
                                optMainWidget->show();
                            else
                                optMainWidget->hide();
                        });

                        // When a button is clicked in the group, update the value in the controller
                        connect(optButtonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, [internalName, this](int id) {
                            _controllersMap[internalName].options[sender()->property("optName").toString()].second = (id == BUTTON_YES_ID);
                        });

                        // Add the button group to the current options map
                        optionsMap.insert(it.key(), optButtonGroup);
                    }

                    // Add all options to the main map
                    _optionsGroupMap.insert(internalName, optionsMap);

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
}

ControllerChoiceWidget::~ControllerChoiceWidget()
{
    // Unload all plugins
    while(!_controllersMap.isEmpty())
        _controllersMap.take(_controllersMap.firstKey()).loader->unload();
}

// Public slots

void ControllerChoiceWidget::selectController(const QString &name)
{
    for(QAbstractButton *button : _buttonGroup->buttons())
    {
        if(button->property(internalNameStr.toStdString().c_str()).toString() == name)
        {
            button->setChecked(true);
            return;
        }
    }
    // Here, no plugin with the specified name was found
    qWarning() << qPrintable(tr("The specified controller \"%1\" doesn't exists !").arg(name));
}

void ControllerChoiceWidget::setOptionForController(const QString &controllerName, const QString &optionName, const bool value)
{
    if(!_optionsGroupMap.contains(controllerName))
    {
        qWarning() << qPrintable(tr("The specified controller \"%1\" doesn't exists !").arg(controllerName));
        return;
    }
    if(!_optionsGroupMap[controllerName].contains(optionName))
    {
        qWarning() << qPrintable(tr("The controller \"%1\" doesn't have an option named \"%2\".").arg(controllerName, optionName));
        return;
    }
    // Check the "yes" or "no" button depending of the value parameter.
    // Only click if the button is not already checked
    if(!_optionsGroupMap[controllerName][optionName]->button(value ? BUTTON_YES_ID : BUTTON_NO_ID)->isChecked())
        _optionsGroupMap[controllerName][optionName]->button(value ? BUTTON_YES_ID : BUTTON_NO_ID)->click();
}

// Getters

QString ControllerChoiceWidget::selectedControllerName()
{
    return _buttonGroup->checkedButton()->property(internalNameStr.toStdString().c_str()).toString();
}

ControllerInterface *ControllerChoiceWidget::selectedController()
{
    // Find the selected plugin
    QPluginLoader *loader = _controllersMap.value(selectedControllerName()).loader;
    // Try to load the plugin
    if(ControllerInterface *plugin = qobject_cast<ControllerInterface *>(loader->instance()))
        return plugin;

    qCritical() << qPrintable(tr("Could not load controller plugin \"%1\" (%2).").arg(selectedControllerName(), loader->errorString()));
    return nullptr;
}

ControllerOptionsList ControllerChoiceWidget::optionsForSelectedController()
{
    return _controllersMap[selectedControllerName()].options;
}

QMap<QString, ControllerWrapper> ControllerChoiceWidget::allControllersWrapper()
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

