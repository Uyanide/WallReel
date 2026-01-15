/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-07 01:12:37
 * @LastEditTime: 2026-01-15 04:07:40
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

#include "version.h"

Q_LOGGING_CATEGORY(logMain, APP_NAME)

static QTextStream* s_logStream = nullptr;
static bool s_isColored         = false;
static QMutex s_logMutex;

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

    QMutexLocker locker(&s_logMutex);

    QString levelTag;
    QString colorTag;
    QString colorText;
    QString resetColor = s_isColored ? "\033[0m" : "";

    switch (type) {
        case QtDebugMsg:
            levelTag = "[DEBUG]";
            colorTag = s_isColored ? "\033[36m" : "";  // Cyan
            break;
        case QtInfoMsg:
            levelTag  = "[INFO]";
            colorTag  = s_isColored ? "\033[92m" : "";  // Light Green
            colorText = s_isColored ? "\033[32m" : "";  // Green
            break;
        case QtWarningMsg:
            levelTag  = "[WARN]";
            colorTag  = s_isColored ? "\033[93m" : "";  // Light Yellow
            colorText = s_isColored ? "\033[33m" : "";  // Yellow
            break;
        case QtCriticalMsg:
            levelTag  = "[ERROR]";
            colorTag  = s_isColored ? "\033[91m" : "";  // Light Red
            colorText = s_isColored ? "\033[31m" : "";  // Red
            break;
        case QtFatalMsg:
            levelTag = "[FATAL]";
            colorTag = s_isColored ? "\033[95m" : "";  // Magenta
            break;
    }

    if (s_logStream) {
        (*s_logStream) << colorTag << levelTag << " " << colorText << msg << resetColor << Qt::endl;
        s_logStream->flush();
    }
}

void Logger::init(FILE* stream) {
    if (stream) {
        delete s_logStream;
        s_logStream = new QTextStream(stream);
    }
    s_isColored = checkIsColored(stream);

    qInstallMessageHandler(messageOutput);
}

void Logger::setLogLevel(QtMsgType level) {
    switch (level) {
        case QtDebugMsg:
            QLoggingCategory::setFilterRules(QString("%1.debug=true").arg(APP_NAME));
            break;
        case QtInfoMsg:
            QLoggingCategory::setFilterRules(QString("%1.debug=false\n%1.info=true").arg(APP_NAME));
            break;
        case QtWarningMsg:
            QLoggingCategory::setFilterRules(QString("%1.debug=false\n%1.info=false\n%1.warning=true").arg(APP_NAME));
            break;
        case QtCriticalMsg:
            QLoggingCategory::setFilterRules(QString("%1.debug=false\n%1.info=false\n%1.warning=false\n%1.critical=true").arg(APP_NAME));
            break;
        case QtFatalMsg:
            QLoggingCategory::setFilterRules(QString("%1.debug=false\n%1.info=false\n%1.warning=false\n%1.critical=false").arg(APP_NAME));
            break;
    }
}

void Logger::quiet() {
    QLoggingCategory::setFilterRules(QString("%1.debug=false\n%1.info=false\n%1.warning=false\n%1.critical=false\n%1.fatal=false").arg(APP_NAME));
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
