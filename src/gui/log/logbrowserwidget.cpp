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

#include "logbrowserwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QTime>
#include <QDebug>

LogBrowserWidget::LogBrowserWidget(QWidget *parent): QWidget(parent)
{
    setWindowTitle(tr("%1 -- Log viewer").arg(APPLICATION_NAME));

    _showDate = false;

    // Main layout
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    _browser = new AutoScrollTextBrowser(this);
    _browser->setAcceptRichText(true);
    layout->addWidget(_browser);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(buttonLayout);

    _checkBoxDate = new QCheckBox(tr("Show date in log"), this);
    _checkBoxDate->setChecked(showDate());
    buttonLayout->addWidget(_checkBoxDate);
    connect(_checkBoxDate, &QCheckBox::clicked, this, &LogBrowserWidget::checkBoxChanged);

    buttonLayout->addStretch(10);

    _clearButton = new QPushButton(this);
    _clearButton->setText(tr("Clear"));
    buttonLayout->addWidget(_clearButton);
    connect(_clearButton, &QPushButton::clicked, _browser, &QTextBrowser::clear);

    _saveButton = new QPushButton(this);
    _saveButton->setText(tr("Save output"));
    buttonLayout->addWidget(_saveButton);
    connect(_saveButton, &QPushButton::clicked, this, &LogBrowserWidget::save);
}

//Getters
bool LogBrowserWidget::showDate() const
{
    return _showDate;
}

//Public slots
void LogBrowserWidget::checkBoxChanged()
{
    setShowDate(_checkBoxDate->isChecked());
}

void LogBrowserWidget::outputMessage(const QString &msg)
{
    QString colored(msg);
    // Set the correct color for messages:
    //   Debug: black
    //   Warning: orange
    //   Critical: red
    //   Fatal: darkred
    if(msg.startsWith(QStringLiteral("Warning")))
    {
        colored.insert(8, "</font></b>");
        colored.insert(0, QStringLiteral("<b><font color=\"orange\">"));
    }
    else if(msg.startsWith(QStringLiteral("Critical")))
    {
        colored.insert(9, "</font></b>");
        colored.insert(0, QStringLiteral("<b><font color=\"red\">"));
    }
    else if(msg.startsWith(QStringLiteral("Fatal")))
    {
        colored.append("</font></b>");
        colored.insert(0, QStringLiteral("<b><font color=\"darkred\">"));
    }
    else if(msg.startsWith(QStringLiteral("Debug")))
    {
        colored.insert(6, "</font></b>");
        colored.insert(0, QStringLiteral("<b><font color=\"black\">"));
    }

    // Insert date after the "<font>" tag
    if(_showDate)
        colored.insert(colored.indexOf("\">") + 2, QString("[%1] ").arg(QTime::currentTime().toString(QStringLiteral("hh:mm:ss:zzz"))));

    _browser->append(colored);
}

void LogBrowserWidget::setShowDate(bool shown)
{
    _showDate = shown;
    _checkBoxDate->setChecked(_showDate);
    emit showDateChanged(shown);
}

//Private slots
void LogBrowserWidget::save()
{
    if(_browser->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, tr("Save error"), tr("Logs are empty.\nYou couldn't save it!"));
        return;
    }

    QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save Log Output"), QString("%1/logfile.log").arg(QDir::homePath()),
                tr("Log Files (*.log);;All Files (*)"));

    if(saveFileName.isEmpty())
        return;

    QFile file(saveFileName);
    if(!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Error"),
                    QString(tr("<nobr>File '%1'<br/>cannot be opened for writing.<br/><br/>"
                               "The log output could <b>not</b> be saved!</nobr>")).arg(saveFileName));
        return;
    }

    QTextStream stream(&file);
    stream << _browser->toPlainText();
    file.close();
}
