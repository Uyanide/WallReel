/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-07 01:12:37
 * @LastEditTime: 2026-01-15 02:10:15
 * @Description: Implementation of logger.
 */
#include "logger.h"

#include <unistd.h>

#include <QCoreApplication>
#include <QDateTime>
#include <QMutex>
#include <QProcessEnvironment>
#include <QTextStream>
#include <cstdio>

Q_LOGGING_CATEGORY(logMain, "wallpaper.carousel")

static QTextStream* g_logStream = nullptr;
static bool g_isColored         = false;
static QMutex g_logMutex;
static const QString g_appName = "wallpaper.carousel";  // same as above

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
        (*g_logStream) << colorTag << levelTag << " " << colorText << msg << resetColor << Qt::endl;
        g_logStream->flush();
    }
}

void Logger::init(FILE* stream) {
    if (stream) {
        delete g_logStream;
        g_logStream = new QTextStream(stream);
    }
    g_isColored = checkIsColored(stream);

    qInstallMessageHandler(messageOutput);
}

void Logger::setLogLevel(QtMsgType level) {
    switch (level) {
        case QtDebugMsg:
            QLoggingCategory::setFilterRules(QString("%1.debug=true").arg(g_appName));
            break;
        case QtInfoMsg:
            QLoggingCategory::setFilterRules(QString("%1.debug=false\n%1.info=true").arg(g_appName));
            break;
        case QtWarningMsg:
            QLoggingCategory::setFilterRules(QString("%1.debug=false\n%1.info=false\n%1.warning=true").arg(g_appName));
            break;
        case QtCriticalMsg:
            QLoggingCategory::setFilterRules(QString("%1.debug=false\n%1.info=false\n%1.warning=false\n%1.critical=true").arg(g_appName));
            break;
        case QtFatalMsg:
            QLoggingCategory::setFilterRules(QString("%1.debug=false\n%1.info=false\n%1.warning=false\n%1.critical=false").arg(g_appName));
            break;
    }
}

void Logger::quiet() {
    QLoggingCategory::setFilterRules(QString("%1.debug=false\n%1.info=false\n%1.warning=false\n%1.critical=false\n%1.fatal=false").arg(g_appName));
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
