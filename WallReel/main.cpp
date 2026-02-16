#include <qtypes.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QQmlApplicationEngine>
#include <QStandardPaths>
#include <QTextStream>

#include "Core/configmgr.hpp"
#include "Core/imagemodel.hpp"
#include "Core/imageprovider.hpp"
#include "Core/utils/logger.hpp"
#include "Core/utils/misc.hpp"
#include "version.h"

/**
 * @brief A static & single-instance class to handle application options.
 *
 */
static class AppOptions {
    QCommandLineParser parser{};

    // The following 3 functions handle specific command line options
    // and mark doReturn as true to indicate that the application should exit
    // after parsing arguments.

    // -v --version
    void printVersion() {
        QTextStream out(stdout);
        out << APP_NAME << " version " << APP_VERSION << Qt::endl;
        doReturn = true;
    }

    // -h --help
    void printHelp() {
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

    // Print error message and help
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
    bool doReturn     = false;  ///< Indicates whether the application should exit after parsing arguments.

    void parseArgs(QApplication* a) {
        parser.setApplicationDescription("A small wallpaper utility made with Qt");

        const QCommandLineOption helpOption    = parser.addHelpOption();
        const QCommandLineOption versionOption = parser.addVersionOption();

        QCommandLineOption verboseOption(QStringList() << "V" << "verbose", "Set log level to DEBUG (default is INFO)");
        parser.addOption(verboseOption);

        QCommandLineOption quietOption(QStringList() << "q" << "quiet", "Suppress all log output");
        parser.addOption(quietOption);

        QCommandLineOption appendDirOption(QStringList() << "d" << "append-dir", "Append an additional wallpaper search directory", "dir");
        parser.addOption(appendDirOption);

        QCommandLineOption configFileOption(QStringList() << "c" << "config-file", "Specify a custom configuration file", "file");
        parser.addOption(configFileOption);

        // Not parser.process(a->arguments()) because we want to handle exit logics ourselves.
        // parser.process(...) will do something like exit(...) that will terminate
        // the application brutally and produce unwanted warnings.
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
            // Default to INFO level
            Logger::setLogLevel(QtDebugMsg);
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

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    a.setApplicationName(APP_NAME);
    a.setApplicationVersion(APP_VERSION);

    Logger::init();
    s_options.parseArgs(&a);

    if (s_options.doReturn) {
        return s_options.errorText.isEmpty() ? 0 : 1;
    }

    Config config(
        ::getConfigDir(),
        s_options.appendDirs,
        s_options.configPath,
        &a);
    QQmlApplicationEngine engine;

    ImageProvider* imageProvider = new ImageProvider();
    engine.addImageProvider(QLatin1String("processed"), imageProvider);

    ImageModel imageModel(
        imageProvider,
        config.getSortConfig(),
        config.getFocusImageSize(),
        &a);

    qmlRegisterSingletonInstance(
        COREMODULE_URI,
        MODULE_VERSION_MAJOR,
        MODULE_VERSION_MINOR,
        "Config",
        &config);
    qmlRegisterSingletonInstance(
        COREMODULE_URI,
        MODULE_VERSION_MAJOR,
        MODULE_VERSION_MINOR,
        "ImageModel",
        &imageModel);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &a,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule(UIMODULE_URI, "Main");

    imageModel.loadAndProcess(config.getWallpapers());

    return a.exec();
}
