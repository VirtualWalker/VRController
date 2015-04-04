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

#include "aboutdialog.h"
#include "../core/utility.h"
#include "../core/licenses.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QPushButton>
#include <QPixmap>

AboutDialog::AboutDialog(QList<LicenseObject> othersLicenses, QWidget *parent): QDialog(parent)
{
    setWindowTitle(tr("About %1").arg(APPLICATION_NAME));
    setAttribute(Qt::WA_DeleteOnClose);

    QTabWidget *tabs = new QTabWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    // Create the license tab
    QTextEdit *licenseTab = new QTextEdit(tabs);
    licenseTab->setReadOnly(true);
    licenseTab->setAlignment(Qt::AlignCenter);
    QString licenseText = FileUtil::readFile(QStringLiteral(":/LICENSE.html"));
    licenseText.replace(QStringLiteral("style=\"text-align: center;\""), QStringLiteral("align=\"center\""));
    licenseTab->setHtml(licenseText);

    // Create the thirdparty tab
    QTextBrowser *thirdLicenseTab = new QTextBrowser(tabs);
    thirdLicenseTab->setReadOnly(true);
    thirdLicenseTab->setOpenExternalLinks(true);
    thirdLicenseTab->setLineWrapMode(QTextBrowser::WidgetWidth);

    // Create 3rd licenses
    for(int i=0, end=ThirdPartiesLicenses::licenses.size(); i<end; ++i)
        thirdLicenseTab->append(licenseToString(ThirdPartiesLicenses::licenses[i]));
    // Add licenses from plugins
    for(int i=0, end=othersLicenses.size(); i<end; ++i)
        thirdLicenseTab->append(licenseToString(othersLicenses[i]));
    thirdLicenseTab->moveCursor(QTextCursor::Start);

    // Create the main tab
    QWidget *aboutTab = new QWidget(tabs);

    QLabel *icon = new QLabel(aboutTab);
    QPixmap pixmap(":/icon.png");
    icon->setPixmap(pixmap.scaled(128, 128, Qt::KeepAspectRatio));

    QLabel *text = new QLabel(aboutTab);
    text->setOpenExternalLinks(true);
    text->setText(QStringLiteral(
                      "<h3>%1</h3>"
                      "<p>Copyright (c) 2015 Fabien Caylus</p>"
                      "<p><b>%3</b></p>"
                      "<p><tt><a href=\"%2\">%2</a></tt></p>"
                  ).arg(QLatin1String(APPLICATION_NAME))
                  .arg(QStringLiteral("https://github.com/Toutjuste/VRController"))
                  .arg(tr("A controller for virtual reality movements with Bluetooth.")));

    QHBoxLayout *hboxLayout = new QHBoxLayout(aboutTab);
    hboxLayout->addWidget(icon);
    hboxLayout->addWidget(text);

    tabs->addTab(aboutTab, tr("&About %1").arg(APPLICATION_NAME));
    tabs->addTab(licenseTab, tr("&License"));
    tabs->addTab(thirdLicenseTab, tr("3rd &party licenses"));

    QPushButton *okButton = new QPushButton(tr("OK"), this);
    connect(okButton, &QPushButton::clicked, this, &AboutDialog::accept);

    mainLayout->addWidget(tabs);
    mainLayout->addWidget(okButton);
}

// Private
QString AboutDialog::licenseToString(LicenseObject license)
{
    if(!license.fromAPlugin)
    {
        return QStringLiteral("<h3>%1 (<a href=\"%2\">%2</a>)</h3><pre>%3</pre><br/><br/>")
                .arg(license.name)
                .arg(license.url)
                .arg(license.license);
    }
    // Else, the license came from a plugin
    return QStringLiteral("<h3>%1 (<a href=\"%2\">%2</a>)</h3><h4>%4</h4><pre>%3</pre><br/><br/>")
            .arg(license.name)
            .arg(license.url)
            .arg(license.license)
            .arg(tr("Used in the controller: %1").arg(license.pluginName));
}



