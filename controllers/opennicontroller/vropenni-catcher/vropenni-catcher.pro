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

##################################################################
# Project file for the separate process used to grab OpenNI data #
##################################################################

# Default defines
APPLICATION_TARGET = vropenni-catcher
APPLICATION_NAME = VROpenNI-Catcher

# Add defines to compiler
DEFINES += "APPLICATION_TARGET=\\\"$${APPLICATION_TARGET}\\\"" \
    "APPLICATION_NAME=\\\"$${APPLICATION_NAME}\\\""

# The executable name
TARGET = $$quote($${APPLICATION_TARGET})

QT += core dbus network
CONFIG += c++11
TEMPLATE = app

# Add OpenNI libs
LIBS += -lOpenNI
INCLUDEPATH += /usr/include/ni
DEFINES += linux
QMAKE_CXXFLAGS += -Wno-unknown-pragmas

# Check for 32 bits systems and add the correct define for OpenNI
linux-g++-32|linux-g++:!contains($$system(uname -m), x86_64) {
    DEFINES += i386
}

BUILD_PATH = build
BIN_PATH = ../../../app/bin
BUILD_STR = debug

CONFIG(debug, debug|release) {
    # Debug
    BUILD_STR = debug
    DEFINES += CORE_DEBUG
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

DBUS_ADAPTORS += openniapplication.xml

SOURCES += \
    src/openniapplication.cpp \
    src/main.cpp

HEADERS += \
    src/openniapplication.h \
    src/openniutil.h \
    src/usbcontroller.h \
    src/openniworker.h \
    src/openniapplicationdefines.h

# French translation
TRANSLATIONS += \
    $$PWD/i18n/$${APPLICATION_TARGET}_fr.ts

# Add qt-unix-signals files
include(thirdparty/qt-unix-signals/sigwatch.pri)
