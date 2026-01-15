/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-05 00:37:58
 * @LastEditTime: 2026-01-15 01:43:14
 * @Description: Entry point.
 */
#include <qapplication.h>
#include <qlogging.h>

#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>

#include "config.h"
#include "logger.h"
#include "main_window.h"

static QString getConfigDir() {
    auto configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (configDir.isEmpty()) {
        configDir = QDir::homePath() + QDir::separator() + ".config" + QDir::separator() + "wallpaper_chooser";
    }
    QDir().mkpath(configDir);
    return configDir;
}

static void setLogLevel(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        QString arg(argv[i]);
        if (arg == "--debug" || arg == "-vvv" || arg == "--verbose") {
            Logger::setLogLevel(QtDebugMsg);
            return;
        } else if (arg == "--info" || arg == "-vv") {
            Logger::setLogLevel(QtInfoMsg);
            return;
        } else if (arg == "--warn" || arg == "-v") {
            Logger::setLogLevel(QtWarningMsg);
            return;
        } else if (arg == "--critical") {
            Logger::setLogLevel(QtCriticalMsg);
            return;
        } else if (arg == "--fatal") {
            Logger::setLogLevel(QtFatalMsg);
            return;
        } else if (arg == "--quiet") {
            Logger::quiet();
            return;
        }
    }
    Logger::setLogLevel(QtInfoMsg);
}

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    Logger::init(stderr);
    setLogLevel(argc, argv);

    Config config(getConfigDir());

    MainWindow w(config);
    w.show();

    return a.exec();
}
