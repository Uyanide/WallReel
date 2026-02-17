#pragma once

#include <QCommandLineParser>
#include <QStringList>

class QApplication;

namespace WallReel::Core {

/**
 * @brief A class to handle application options.
 */
class AppOptions {
    QCommandLineParser parser;

    // -v --version
    void printVersion();

    // -h --help
    void printHelp();

    // Print error message and help
    void printError();

  public:
    QString configPath;
    QStringList appendDirs;
    QString errorText;
    bool doReturn = false;  ///< Indicates whether the application should exit after parsing arguments.

    AppOptions();
    void parseArgs(QApplication& app);
};

}  // namespace WallReel::Core
