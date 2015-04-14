#############################################################################
##
## This file is part of VRController.
## Copyright (c) 2015 Fabien Caylus <toutjuste13@gmail.com>
##
## This file is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This file is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##
#############################################################################

########################################
# Project file for the fake controller #
########################################

CONTROLLER_NAME = fakecontroller
APP_PATH = ../../app
include($$PWD/../controllerscommon.pri)

QT += gui widgets

SOURCES += \
    src/fakecontrollerwidget.cpp \
    ../../app/src/commonwidgets/dial.cpp

HEADERS += \
    src/fakecontrollerwidget.h \
    ../../app/src/commonwidgets/dial.h

# French translation
TRANSLATIONS += \
    $$PWD/i18n/$${CONTROLLER_NAME}_fr.ts
