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

#include "openniworker.h"

#include <QCoreApplication>
#include <QThread>
#include <QMessageLogContext>
#include <iostream>
#include "sigwatch.h"

const QString separator = "%";

static bool firstSensor;

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString output = "##//##";

    // Output in a custom format
    // All informations are separated by "##//##"

    // Check for the specical command
    if(msg == "*****started*****")
    {
        std::fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
        return;
    }

    switch(type)
    {
        case QtDebugMsg:
            output += "D";
            break;
        case QtWarningMsg:
            output += "W";
            break;
        case QtCriticalMsg:
            output += "C";
            break;
        case QtFatalMsg:
            output += "F";
            break;
    }

    output.append(separator);
    output.append(msg);
    output.append(separator);
    output.append(context.file);
    output.append(separator);
    output.append(context.line);
    output.append(separator);
    output.append(context.function);
    output.append(separator);
    output.append(QObject::tr("Sensor %1").arg(firstSensor ? 0 : 1));

    // Output to standard error channel
    std::fprintf(stderr, "%s\n", output.toLocal8Bit().constData());

    // Quit if it's a fatal message
    if(type == QtFatalMsg)
        abort();
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageOutput);

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setOrganizationName(APPLICATION_NAME);

    // Listen for unix signals
    UnixSignalWatcher sigWatch;
    sigWatch.watchForSignal(SIGTERM);
    QObject::connect(&sigWatch, &UnixSignalWatcher::unixSignal, &app, &QCoreApplication::quit);

    // Arguments must be:
    //   frequency useAKinect firstSensor camVendor camProduct camBus camDevice motorVendor motorProduct motorBus motorDevice
    const QStringList arguments = app.arguments();
    if(arguments.size() != 12)
        return EXIT_FAILURE;

    firstSensor = arguments.at(3) == "true" ? true : false;

    OpenNIWorker *openniWorker;
    QThread openniThread;
    openniWorker = new OpenNIWorker(arguments.at(1).toInt(),
                                    arguments.at(2) == "true" ? true : false,
                                    firstSensor,
                                    USBDevicePath({arguments.at(4).toInt(),
                                                   arguments.at(5).toInt(),
                                                   arguments.at(6).toInt(),
                                                   arguments.at(7).toInt()}),
                                    USBDevicePath({arguments.at(8).toInt(),
                                                   arguments.at(9).toInt(),
                                                   arguments.at(10).toInt(),
                                                   arguments.at(11).toInt()}));

    QObject::connect(&openniThread, &QThread::finished, openniWorker, &QObject::deleteLater);
    QObject::connect(&openniThread, &QThread::started, openniWorker, &OpenNIWorker::launch);

    // D-Bus part
    OpenNIWorkerAdaptator *openniWorkerAdaptator = new OpenNIWorkerAdaptator(openniWorker);
    new OpenNIApplicationInterfaceAdaptor(openniWorkerAdaptator);
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerObject(DBUS_ROOT_OBJECT, openniWorkerAdaptator);
    connection.registerService(firstSensor ? DBUS_SERVICE_NAME_1 : DBUS_SERVICE_NAME_2);


    openniWorker->moveToThread(&openniThread);
    openniThread.start();

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
        openniWorkerAdaptator->deleteLater();
        openniWorker->requestStop();
        openniThread.quit();
        openniThread.wait();
    });

    return app.exec();
}
