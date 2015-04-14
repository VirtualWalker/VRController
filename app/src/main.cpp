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

#include "gui/mainwindow.h"
#include "gui/log/logbrowser.h"

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageLogContext>
#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QString>
#include <QStringList>

LogBrowser *globalLogBrowser;

void messageOutput(QtMsgType msgType, const QMessageLogContext &logContext, const QString &message)
{
    QString msg = message;
    QMessageLogContext *context;
    QtMsgType type = msgType;

    // Check for messages from sub-processes
    if(msg.startsWith("##//##"))
    {
        msg.remove(0, 6);
        msg.remove("\n");
        const QStringList infos = msg.split("%");
        if(infos.size() == 6)
        {
            if(infos[0] == "D")
                type = QtDebugMsg;
            else if(infos[0] == "W")
                type = QtWarningMsg;
            else if(infos[0] == "C")
                type = QtCriticalMsg;
            else if(infos[0] == "F")
                type = QtFatalMsg;

            context = new QMessageLogContext(infos[2].toStdString().c_str(), infos[3].toInt(),
                    infos[4].toStdString().c_str(), "");

            msg = QObject::tr("From %1: %2").arg(infos[5]).arg(infos[1]);
        }
        else
             context = new QMessageLogContext(logContext.file, logContext.line, logContext.function, logContext.category);
    }
    else
        context = new QMessageLogContext(logContext.file, logContext.line, logContext.function, logContext.category);

    QString output;

#ifdef QT_DEBUG
    switch(type)
    {
        case QtDebugMsg:
            output = QString("Debug: %0 (%1:%2, %3)").arg(msg).arg(context->file).arg(context->line).arg(context->function);
            break;
        case QtWarningMsg:
            output = QString("Warning: %0 (%1:%2, %3)").arg(msg).arg(context->file).arg(context->line).arg(context->function);
            break;
        case QtCriticalMsg:
            output = QString("Critical: %0 (%1:%2, %3)").arg(msg).arg(context->file).arg(context->line).arg(context->function);
            break;
        case QtFatalMsg:
            output = QString("Fatal /!\\: %0 (%1:%2, %3)").arg(msg).arg(context->file).arg(context->line).arg(context->function);
            break;
        default:
            output = QString("Unknown: %0 (%1:%2, %3)").arg(msg).arg(context->file).arg(context->line).arg(context->function);
            break;
    }
    // Output to standard error channel
    fprintf(stderr, "%s\n", output.toLocal8Bit().constData());
#else
    switch(type)
    {
        case QtDebugMsg:
            output = QString("Debug: %0").arg(msg);
            break;
        case QtWarningMsg:
            output = QString("Warning: %0").arg(msg);
            break;
        case QtCriticalMsg:
            output = QString("Critical: %0").arg(msg);
            break;
        case QtFatalMsg:
            output = QString("Fatal /!\\: %0").arg(msg);
            break;
        default:
            output = QString("Unknown: %0").arg(msg);
            break;
    }
    // Don't output when on debug messages (only in the GUI, not in the console)
    if(type != QtDebugMsg)
        fprintf(stderr, "%s\n", output.toLocal8Bit().constData());
#endif

    delete context;
    context = nullptr;

    // Quit if it's a fatal message
    if(type == QtFatalMsg)
        abort();

    // Add output to log widget
    if(globalLogBrowser)
        globalLogBrowser->outputMessage(output);
}

void loadTranslations(const QString& path, const QString &locale, QApplication *app)
{
    QDirIterator iterator(path, QStringList() << "*_" + locale + ".qm", QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        iterator.next();
        QTranslator *translator = new QTranslator(app);
        translator->load(iterator.fileName(), iterator.fileInfo().canonicalPath());
        app->installTranslator(translator);
    }
}

// Return the int value in the options.
// If not set, return -1
int intFromParser(QCommandLineParser& parser, const QString& optName)
{
    bool ok = false;
    const int number = parser.value(optName).toInt(&ok);
    return ok ? number : -1;
}

int main(int argc, char *argv[])
{
    // Install the custom handler
    qInstallMessageHandler(messageOutput);

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setOrganizationName(APPLICATION_NAME);
    QCoreApplication::setApplicationVersion("0.0.1-alpha");

    // Load default Qt translations
    const QString locale = QLocale::system().name().section('_', 0, 0);
    QTranslator qtTranslator;
    qtTranslator.load(QStringLiteral("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    // Load translations in the "translations" dir
    loadTranslations(QCoreApplication::applicationDirPath() + QStringLiteral("/translations"), locale, &app);

    // Check the parameters
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::tr("Simple application that allow the control of an Android VR app with the Bluetooth and a 3D depth sensor."));
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption(QCommandLineOption({"a", "auto-start"}, QCoreApplication::translate("options", "Auto start the bluetooth listening (don't wait for the user click).")));
    parser.addOption(QCommandLineOption({"c", "controller"}, QCoreApplication::translate("options", "The controller <controller-name> will be used."), QCoreApplication::translate("options", "controller-name")));
    parser.addOption(QCommandLineOption({"p", "port"}, QCoreApplication::translate("options", "The Bluetooth engine will listen on the specified <port-number>. The <port-number> must be in range 1-30. Set to 0 if you want to select the first available."), QCoreApplication::translate("options", "port-number")));
    parser.addOption(QCommandLineOption({"f", "frequency"}, QCoreApplication::translate("options", "Frequency for emitting data to the bluetooth device (number of data per second)"), QCoreApplication::translate("options", "number-per-second")));
    parser.addOption(QCommandLineOption("nologwidget", QCoreApplication::translate("options", "Don't show the log console in the bottom of the window.")));

    parser.process(app);

    const bool useLogWidget = !parser.isSet("nologwidget");
    if(useLogWidget)
        globalLogBrowser = new LogBrowser();

    MainWindow window(globalLogBrowser, parser.isSet("auto-start"), parser.value("controller"), intFromParser(parser, "port"), intFromParser(parser, "frequency"));
    window.show();

    // Execute the main loop
    const int result = app.exec();

    // Delete the log browser if needed
    if(useLogWidget)
        delete globalLogBrowser;

    return result;
}
