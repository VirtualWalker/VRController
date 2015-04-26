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

###########################
# Main Configuration file #
###########################

# Default defines
APPLICATION_TARGET = vrcontroller
APPLICATION_NAME = VRController

# Add defines to compiler
DEFINES += "APPLICATION_TARGET=\\\"$${APPLICATION_TARGET}\\\"" \
    "APPLICATION_NAME=\\\"$${APPLICATION_NAME}\\\""

# Check if we are in a NO_GUI build
CONFIG(NO_GUI) {
    DEFINES += NO_GUI
}

# Check if we don't need the bluetooth
CONFIG(NO_BLUETOOTH) {
    DEFINES += NO_BLUETOOTH
}

# The executable name
TARGET = $$quote($${APPLICATION_TARGET})

QT += core gui widgets
CONFIG += c++11
TEMPLATE = app

# Add bluetooth library
LIBS += -lbluetooth

BUILD_PATH = build
BIN_PATH = bin
BUILD_STR = debug

CONFIG(debug, debug|release) {
    # Debug
    BUILD_STR = debug
    DEFINES += CORE_DEBUG
    TARGET = $$join(TARGET,,,d)
    CONFIG += warn_on
}
else {
    # Release
    BUILD_STR = release
    DEFINES += CORE_RELEASE
    CONFIG += warn_off
}

# Build and bin paths
OBJECTS_DIR = $${BUILD_PATH}/$${BUILD_STR}/obj
UI_DIR = $${BUILD_PATH}/$${BUILD_STR}/ui
MOC_DIR = $${BUILD_PATH}/$${BUILD_STR}/moc
RCC_DIR = $${BUILD_PATH}/$${BUILD_STR}/rcc

DESTDIR = $${BIN_PATH}/$${BUILD_STR}

SOURCES += \
    src/main.cpp \
    src/gui/mainwindow.cpp \
    src/gui/listeningwidget.cpp \
    src/gui/log/logbrowser.cpp \
    src/gui/log/logbrowserwidget.cpp \
    src/gui/aboutdialog.cpp \
    src/commonwidgets/autoscrolltextbrowser.cpp \
    src/commonwidgets/dial.cpp \
    src/gui/controllerchoicewidget.cpp \
    src/commonwidgets/hintdialog.cpp

HEADERS += \
    src/core/bluetoothmanager.h \
    src/gui/mainwindow.h \
    src/gui/listeningwidget.h \
    src/gui/log/logbrowser.h \
    src/gui/log/logbrowserwidget.h \
    src/gui/aboutdialog.h \
    src/core/licenses.h \
    src/commonwidgets/autoscrolltextbrowser.h \
    src/commonwidgets/dial.h \
    src/interfaces/controllerinterface.h \
    src/interfaces/controllercommon.h \
    src/gui/controllerchoicewidget.h \
    src/commonwidgets/hintdialog.h \
    src/core/utility.h

OTHER_FILES += \
    src/interfaces/ControllerInterface \
    src/commonwidgets/Dial \
    src/commonwidgets/AutoScrollTextBrowser

# Add QProgressIndicator files
include($$PWD/../thirdparty/qprogressindicator/qprogressindicator.pri)

# French translation
TRANSLATIONS += \
    $$PWD/i18n/$${APPLICATION_TARGET}_fr.ts

RESOURCES += \
    $$PWD/../resources.qrc

