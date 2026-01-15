/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-05 00:37:58
 * @LastEditTime: 2026-01-15 03:41:55
 * @Description: Argument parser and entry point.
 */
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>

#include "config.h"
#include "logger.h"
#include "main_window.h"
#include "utils.h"
#include "version.h"

static class AppOptions {
    QCommandLineParser parser{};

    void printVersion() {
        QTextStream out(stdout);
        out << APP_NAME << " version " << APP_VERSION << Qt::endl;
        doReturn = true;
    }

    void printHelp() {
        QTextStream out(stdout);
        out << parser.helpText() << Qt::endl;
        doReturn = true;
    }

    void printError() {
        if (!errorText.isEmpty()) {
            QTextStream out(stderr);
            out << errorText << Qt::endl;
            printHelp();
        }
        doReturn = true;
    }

  public:
    QString configPath = "";
    QStringList appendDirs;
    QString errorText = "";
    bool doReturn     = false;

    void parseArgs(QApplication* a) {
        parser.setApplicationDescription("A small wallpaper utility made with Qt");

        const QCommandLineOption helpOption    = parser.addHelpOption();
        const QCommandLineOption versionOption = parser.addVersionOption();

        QCommandLineOption verboseOption(QStringList() << "V" << "verbose", "Set log level to DEBUG");
        parser.addOption(verboseOption);

        QCommandLineOption quietOption(QStringList() << "q" << "quiet", "Suppress all log output");
        parser.addOption(quietOption);

        QCommandLineOption appendDirOption(QStringList() << "d" << "append-dir", "Append an additional wallpaper search directory", "dir");
        parser.addOption(appendDirOption);

        QCommandLineOption configFileOption(QStringList() << "c" << "config-file", "Specify a custom configuration file", "file");
        parser.addOption(configFileOption);

        if (!parser.parse(a->arguments())) {
            errorText = parser.errorText();
            doReturn  = true;
            return;
        }

        if (parser.isSet(versionOption)) {
            printVersion();
            return;
        }

        if (parser.isSet(helpOption)) {
            printHelp();
            return;
        }

        if (parser.isSet(verboseOption)) {
            Logger::setLogLevel(QtDebugMsg);
        } else if (parser.isSet(quietOption)) {
            Logger::quiet();
        } else {
            Logger::setLogLevel(QtInfoMsg);
        }

        for (const QString& dir : parser.values(appendDirOption)) {
            if (checkDir(dir)) {
                appendDirs.append(dir);
            } else {
                errorText = QString("Error: Directory does not exist or is not accessible: %1").arg(dir);
                printError();
                return;
            }
        }

        if (parser.isSet(configFileOption)) {
            QString path = parser.value(configFileOption);
            if (checkFile(path)) {
                configPath = path;
            } else {
                errorText = QString("Error: Config file does not exist or is not accessible: %1").arg(path);
                printError();
                return;
            }
        }
    }

} s_options;

static QString getConfigDir() {
    auto configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (configDir.isEmpty()) {
        configDir = QDir::homePath() + QDir::separator() + ".config" + QDir::separator() + APP_NAME;
    }
    QDir().mkpath(configDir);
    return configDir;
}

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    a.setApplicationName(APP_NAME);
    a.setApplicationVersion(APP_VERSION);

    Logger::init();
    s_options.parseArgs(&a);

    if (s_options.doReturn) {
        return s_options.errorText.isEmpty() ? 0 : 1;
    }

    Config config(getConfigDir(), s_options.appendDirs, s_options.configPath, &a);

    MainWindow w(config);
    w.show();

    return a.exec();
}
