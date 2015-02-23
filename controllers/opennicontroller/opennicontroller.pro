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

##########################################
# Project file for the OpenNI controller #
##########################################

CONTROLLER_NAME = opennicontroller
include($$PWD/../controllerscommon.pri)

QT += gui widgets opengl

# Add OpenCV libs
LIBS += \
    -lopencv_core \
    -lopencv_imgproc

INCLUDEPATH += /usr/include/

# Add OpenNI libs
LIBS += -lOpenNI
INCLUDEPATH += /usr/include/ni
DEFINES += linux
QMAKE_CXXFLAGS += -Wno-unknown-pragmas

# Check for 32 bits systems and add the correct define for OpenNI
linux-g++-32|linux-g++:!contains($$system(uname -m), x86_64) {
    DEFINES += i386
}

SOURCES += \
    src/opennicontrollerwidget.cpp \
    src/opencvutil.cpp \
    src/openniapplication.cpp \
    src/opencvwidget.cpp

HEADERS += \
    src/opennicontrollerwidget.h \
    src/opencvutil.h \
    src/openniapplication.h \
    src/opencvwidget.h \
    src/openniutil.h \
    ../../app/src/core/utility.h \
    src/usbcontroller.h

RESOURCES += \
    $$PWD/resources.qrc

# French translation
TRANSLATIONS += \
    $$PWD/i18n/$${CONTROLLER_NAME}_fr.ts
