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

#ifndef THIRDPARTIESLICENSES_H
#define THIRDPARTIESLICENSES_H

#include <QString>
#include <QList>

// This file contains licenses about 3rd parties program used in VRController
// All copyright notices are only available for these 3rd parties program.
// Each controller can add custom licenses if needed
struct LicenseObject {
    QString name;
    QString url;
    QString license;
    bool fromAPlugin;
    QString pluginName;

    LicenseObject(): fromAPlugin(false) {}
    LicenseObject(QString name, QString url, QString license)
        : name(name), url(url), license(license), fromAPlugin(false) {}
    LicenseObject(QString name, QString url, QString license, QString pluginName)
        : name(name), url(url), license(license), fromAPlugin(true), pluginName(pluginName) {}
};

typedef QList<LicenseObject> LicenseObjectList;

namespace ThirdPartiesLicenses
{
    static QString licenseQProgressIndicator =
            "Copyright (C) 2009 - 2010 Morgan Leborgne\n"
            "\n"
            "This program is free software: you can redistribute it and/or modify\n"
            "it under the terms of the GNU Lesser General Public License as published by\n"
            "the Free Software Foundation, either version 3 of the License, or\n"
            "(at your option) any later version.\n"
            "\n"
            "This program is distributed in the hope that it will be useful,\n"
            "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
            "GNU Lesser General Public License for more details.\n"
            "\n"
            "You should have received a copy of the GNU Lesser General Public License\n"
            "along with QProgressIndicator. If not, see &lt;http://www.gnu.org/licenses/&gt;\n";


    static const LicenseObjectList licenses = {
        LicenseObject(QStringLiteral("QProgressIndicator"), QStringLiteral("https://github.com/mojocorp/QProgressIndicator"), licenseQProgressIndicator)
    };
}


#endif // THIRDPARTIESLICENSES_H
