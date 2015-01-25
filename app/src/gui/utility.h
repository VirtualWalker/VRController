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

#ifndef UTILITY_H
#define UTILITY_H

//
// This file contains some utility methods
//

#include <QString>
#include <QTextStream>
#include <QFile>

namespace FileUtil
{
    QString readFile(const QString &path)
    {
        QFile file(path);
        if(!file.open(QFile::ReadOnly | QFile::Text))
            return QString();
        QTextStream in(&file);
        return in.readAll();
    }
}

#endif // UTILITY_H
