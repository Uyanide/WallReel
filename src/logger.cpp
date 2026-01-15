/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-07 01:12:37
 * @LastEditTime: 2026-01-15 00:58:04
 * @Description: Implementation of logger.
 */
#include "logger.h"

#include <unistd.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QMutex>
#include <QProcessEnvironment>
#include <cstdio>

Q_LOGGING_CATEGORY(logMain, "wallpaper.carousel")

static FILE* g_logStream = stderr;
static bool g_isColored  = false;
static QMutex g_logMutex;

static bool checkIsColored(FILE* stream) {
    if (!stream || !isatty(fileno(stream))) {
        return false;
    }
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString term            = env.value("TERM");
    if (term.isEmpty() || term == "dumb") {
        return false;
    }
    return true;
}

static void messageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    Q_UNUSED(context);

    QMutexLocker locker(&g_logMutex);

    QString levelTag;
    QString colorTag;
    QString colorText;
    QString resetColor = g_isColored ? "\033[0m" : "";

    switch (type) {
        case QtDebugMsg:
            levelTag = "[DEBUG]";
            colorTag = g_isColored ? "\033[36m" : "";  // Cyan
            break;
        case QtInfoMsg:
            levelTag  = "[INFO]";
            colorTag  = g_isColored ? "\033[92m" : "";  // Light Green
            colorText = g_isColored ? "\033[32m" : "";  // Green
            break;
        case QtWarningMsg:
            levelTag  = "[WARN]";
            colorTag  = g_isColored ? "\033[93m" : "";  // Light Yellow
            colorText = g_isColored ? "\033[33m" : "";  // Yellow
            break;
        case QtCriticalMsg:
            levelTag  = "[ERROR]";
            colorTag  = g_isColored ? "\033[91m" : "";  // Light Red
            colorText = g_isColored ? "\033[31m" : "";  // Red
            break;
        case QtFatalMsg:
            levelTag = "[FATAL]";
            colorTag = g_isColored ? "\033[95m" : "";  // Magenta
            break;
    }

    if (g_logStream) {
        fprintf(g_logStream, "%s%s %s%s%s\n", qPrintable(colorTag), qPrintable(levelTag), qPrintable(colorText), qPrintable(msg), qPrintable(resetColor));
        fflush(g_logStream);
    }
}

void Logger::init(FILE* stream) {
    if (stream) {
        g_logStream = stream;
    }
    g_isColored = checkIsColored(g_logStream);

    qInstallMessageHandler(messageOutput);
}

void Logger::setLogLevel(QtMsgType level) {
    switch (level) {
        case QtDebugMsg:
            QLoggingCategory::setFilterRules("wallpaper.carousel.debug=true");
            break;
        case QtInfoMsg:
            QLoggingCategory::setFilterRules("wallpaper.carousel.debug=false\nwallpaper.carousel.info=true");
            break;
        case QtWarningMsg:
            QLoggingCategory::setFilterRules("wallpaper.carousel.debug=false\nwallpaper.carousel.info=false\nwallpaper.carousel.warning=true");
            break;
        case QtCriticalMsg:
            QLoggingCategory::setFilterRules("wallpaper.carousel.debug=false\nwallpaper.carousel.info=false\nwallpaper.carousel.warning=false\nwallpaper.carousel.critical=true");
            break;
        case QtFatalMsg:
            QLoggingCategory::setFilterRules("wallpaper.carousel.debug=false\nwallpaper.carousel.info=false\nwallpaper.carousel.warning=false\nwallpaper.carousel.critical=false");
            break;
    }
}

void Logger::quiet() {
    QLoggingCategory::setFilterRules("wallpaper.carousel.debug=false\nwallpaper.carousel.info=false\nwallpaper.carousel.warning=false\nwallpaper.carousel.critical=false\nwallpaper.carousel.fatal=false");
}

void GeneralLogger::debug(const QString& msg) {
    qCDebug(logMain).noquote() << msg;
}

void GeneralLogger::info(const QString& msg) {
    qCInfo(logMain).noquote() << msg;
}

void GeneralLogger::warn(const QString& msg) {
    qCWarning(logMain).noquote() << msg;
}

void GeneralLogger::critical(const QString& msg) {
    qCCritical(logMain).noquote() << msg;
}

void GeneralLogger::fatal(const QString& msg) {
    qCFatal(logMain).noquote() << msg;
}
