#############################################################################
##
## This file is part of VRController.
## Copyright (c) 2014 Fabien Caylus <toutjuste13@gmail.com>
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

# Show an error if we are not in linux since we need the BlueZ library
# To install bluez packages on ubuntu, type:
# $ sudo apt-get install bluez bluetooth libbluetooth-dev
!unix|macx {
    error(You are not in a linux system !)
}

# Default defines
APPLICATION_TARGET = vrcontroller
APPLICATION_NAME = VRController

# Add defines to compiler
DEFINES += "APPLICATION_TARGET=\\\"$${APPLICATION_TARGET}\\\"" \
    "APPLICATION_NAME=\\\"$${APPLICATION_NAME}\\\""

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

SOURCES +=  \
    src/gui/main.cpp \
    src/gui/mainwindow.cpp \
    src/gui/listeningwidget.cpp \
    src/gui/fakecontroller.cpp \
    src/gui/log/logbrowser.cpp \
    src/gui/log/logbrowserwidget.cpp \
    src/gui/autoscrolltextbrowser.cpp \
    src/gui/about.cpp

HEADERS  +=  \
    src/core/bluetoothmanager.h \
    src/gui/mainwindow.h \
    src/gui/listeningwidget.h \
    src/gui/fakecontroller.h \
    src/gui/log/logbrowser.h \
    src/gui/log/logbrowserwidget.h \
    src/gui/autoscrolltextbrowser.h \
    src/gui/about.h \
    src/gui/utility.h \
    src/gui/licenses.h

# Add progress indicator files
include($$PWD/thirdparty/qprogressindicator/qprogressindicator.pri)

# Add french translation
TRANSLATIONS += \
    i18n/$${APPLICATION_TARGET}_fr.ts

RESOURCES += \
    resources.qrc

