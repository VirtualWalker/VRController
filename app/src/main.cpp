/*
 * This file is part of VRController.
 * Copyright (c) 2014 Fabien Caylus <toutjuste13@gmail.com>
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

LogBrowser *globalLogBrowser;

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString output;

#ifdef QT_DEBUG
    switch(type)
    {
        case QtDebugMsg:
            output = QString("Debug: %0 (%1:%2, %3)").arg(msg).arg(context.file).arg(context.line).arg(context.function);
            break;
        case QtWarningMsg:
            output = QString("Warning: %0 (%1:%2, %3)").arg(msg).arg(context.file).arg(context.line).arg(context.function);
            break;
        case QtCriticalMsg:
            output = QString("Critical: %0 (%1:%2, %3)").arg(msg).arg(context.file).arg(context.line).arg(context.function);
            break;
        case QtFatalMsg:
            output = QString("Fatal /!\\: %0 (%1:%2, %3)").arg(msg).arg(context.file).arg(context.line).arg(context.function);
            break;
        default:
            output = QString("Unknown: %0 (%1:%2, %3)").arg(msg).arg(context.file).arg(context.line).arg(context.function);
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

int main(int argc, char *argv[])
{
    // Install the custom handler
    qInstallMessageHandler(messageOutput);
    bool useLogWidget = true;

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(APPLICATION_NAME);
    QCoreApplication::setOrganizationName(APPLICATION_NAME);

    // Load default Qt translations
    const QString locale = QLocale::system().name().section('_', 0, 0);
    QTranslator qtTranslator;
    qtTranslator.load(QStringLiteral("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    // Load translations in the "translations" dir
    loadTranslations(QCoreApplication::applicationDirPath() + QStringLiteral("/translations"), locale, &app);

    // Check log param
    for(int i=1; i < argc; ++i)
    {
        if(strcmp(argv[i], "--nologwidget") == 0)
        {
            qDebug() << qPrintable(QObject::tr("--nologwidget argument detected ! Hide console output."));
            useLogWidget = false;
        }
    }

    if(useLogWidget)
        globalLogBrowser = new LogBrowser();

    MainWindow window(globalLogBrowser);
    window.show();

    // Execute the main loop
    const int result = app.exec();

    // Delete the log browser if needed
    if(useLogWidget)
        delete globalLogBrowser;

    return result;
}
