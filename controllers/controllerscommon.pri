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

#########################################
# Controllers common configuration file #
#########################################

#
# The plugin must define this variable :
# - CONTROLLER_NAME (the name used for this controller in lower letters)
#

# Plugin configuration
TEMPLATE = lib
CONFIG += dll plugin c++11

# Qt modules
QT += core
QT -= gui

# Output name
TARGET = $${CONTROLLER_NAME}

# Check if we are in a NO_GUI build
CONFIG(NO_GUI) {
    DEFINES += NO_GUI
}

BUILD_PATH = build
BIN_PATH = ../../app/bin
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

DESTDIR = $${BIN_PATH}/$${BUILD_STR}/controllers

CUSTOM_INCDEP_PATH = \
    $${_PRO_FILE_PWD_} \
    $${_PRO_FILE_PWD_}/src \
    $$PWD/../app/src \
    $$PWD/../app/src/interfaces \
    $$PWD/../app/src/commonwidgets

INCLUDEPATH += \
    $$CUSTOM_INCDEP_PATH

DEPENDPATH += \
    $$CUSTOM_INCDEP_PATH

HEADERS += \
    src/$${CONTROLLER_NAME}.h \
    ../../app/src/interfaces/ControllerInterface.h

SOURCES += \
    src/$${CONTROLLER_NAME}.cpp

OTHER_FILES += \
    spec.json
