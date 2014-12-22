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

#include "about.h"
#include "utility.h"
#include "licenses.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QPushButton>
#include <QPixmap>

AboutDialog::AboutDialog(QWidget *parent): QDialog(parent)
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
    licenseTab->setPlainText(FileUtil::readFile(QStringLiteral(":/LICENSE")));

    // Create the thirdparty tab
    QTextBrowser *thirdLicenseTab = new QTextBrowser(tabs);
    thirdLicenseTab->setReadOnly(true);
    thirdLicenseTab->setOpenExternalLinks(true);

    // Create 3rd licenses
    for(int i = 0; !ThirdPartiesLicenses::licenses[i].isEmpty(); ++i)
    {
        thirdLicenseTab->append(QStringLiteral("<h3>%1 (<a href=\"%2\">%2</a>)</h3><pre>%3</pre>")
                .arg(QString::fromLatin1(ThirdPartiesLicenses::licenses[i].name))
                .arg(QString::fromLatin1(ThirdPartiesLicenses::licenses[i].url))
                .arg(QString::fromLatin1(ThirdPartiesLicenses::licenses[i].license)));
    }
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
                      "<p>Copyright (c) 2014 Fabien Caylus</p>"
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



