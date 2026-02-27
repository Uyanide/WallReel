#include "appoptions.hpp"

#include <QApplication>
#include <QCommandLineOption>
#include <QTextStream>

#include "Utils/misc.hpp"
#include "logger.hpp"
#include "version.h"

namespace WallReel::Core {

// -v --version
void AppOptions::printVersion() {
    QTextStream out(stdout);
    out << APP_NAME << " version " << APP_VERSION << Qt::endl;
    doReturn = true;
}

// -h --help
void AppOptions::printHelp() {
    QTextStream out(stdout);
    QString helpText = parser.helpText();
    auto lines       = helpText.split('\n');
    for (auto& line : lines) {
        if (line.contains("--help-all")) {
            // Remove the --help-all option line added by Qt by default
            continue;
        }
        out << line << Qt::endl;
    }
    doReturn = true;
}

// -C --clear-cache
void AppOptions::clearCache() {
    QDir cacheDir = Utils::getCacheDir();
    if (cacheDir.exists()) {
        if (cacheDir.removeRecursively()) {
            Logger::info("Cache cleared successfully.");
        } else {
            Logger::warn("Failed to clear cache.");
        }
    } else {
        Logger::info("Cache directory does not exist, nothing to clear.");
    }
    doReturn = true;
}

// Print error message and help
void AppOptions::printError() {
    if (!errorText.isEmpty()) {
        QTextStream out(stderr);
        out << errorText << Qt::endl;
        printHelp();
    }
    doReturn = true;
}

AppOptions::AppOptions() = default;

void AppOptions::parseArgs(QApplication& app) {
    parser.setApplicationDescription("A small wallpaper utility made with Qt");

    const QCommandLineOption helpOption    = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    QCommandLineOption verboseOption(QStringList() << "V" << "verbose", "Set log level to DEBUG (default is INFO)");
    parser.addOption(verboseOption);

    QCommandLineOption clearCacheOption(QStringList() << "C" << "clear-cache", "Clear the image cache and exit");
    parser.addOption(clearCacheOption);

    QCommandLineOption quietOption(QStringList() << "q" << "quiet", "Suppress all log output");
    parser.addOption(quietOption);

    QCommandLineOption appendDirOption(QStringList() << "d" << "append-dir", "Append an additional wallpaper search directory", "dir");
    parser.addOption(appendDirOption);

    QCommandLineOption configFileOption(QStringList() << "c" << "config-file", "Specify a custom configuration file", "file");
    parser.addOption(configFileOption);

    // Not parser.process(a->arguments()) because we want to handle exit logics ourselves.
    // parser.process(...) will do something like exit(...) that will terminate
    // the application brutally and produce unwanted warnings.
    if (!parser.parse(app.arguments())) {
        errorText = parser.errorText();
        doReturn  = true;
        return;
    }

    if (parser.isSet(helpOption)) {
        printHelp();
        return;
    }

    if (parser.isSet(versionOption)) {
        printVersion();
        return;
    }

    if (parser.isSet(clearCacheOption)) {
        clearCache();
        return;
    }

    if (parser.isSet(verboseOption)) {
        Logger::setLogLevel(QtDebugMsg);
    } else if (parser.isSet(quietOption)) {
        Logger::quiet();
    } else {
        // Default to INFO level
        Logger::setLogLevel(QtDebugMsg);
    }

    for (const QString& dir : parser.values(appendDirOption)) {
        if (Utils::checkDir(dir)) {
            appendDirs.append(dir);
        } else {
            errorText = QString("Error: Directory does not exist or is not accessible: %1").arg(dir);
            printError();
            return;
        }
    }

    if (parser.isSet(configFileOption)) {
        QString path = parser.value(configFileOption);
        if (Utils::checkFile(path)) {
            configPath = path;
        } else {
            errorText = QString("Error: Config file does not exist or is not accessible: %1").arg(path);
            printError();
            return;
        }
    }
}

}  // namespace WallReel::Core
