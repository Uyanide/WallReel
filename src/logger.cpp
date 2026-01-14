/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-07 01:12:37
 * @LastEditTime: 2025-12-01 01:12:49
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

static FILE* g_logStream                    = stderr;
static GeneralLogger::LogIndent g_maxIndent = GeneralLogger::DETAIL;
static bool g_isColored                     = false;
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

void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
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

void Logger::init(FILE* stream, GeneralLogger::LogIndent maxIndent) {
    if (stream) {
        g_logStream = stream;
    }
    g_maxIndent = maxIndent;
    g_isColored = checkIsColored(g_logStream);

    qInstallMessageHandler(myMessageOutput);
}

void Logger::setMaxIndent(GeneralLogger::LogIndent indent) {
    g_maxIndent = indent;
}

GeneralLogger::LogIndent Logger::maxIndent() {
    return g_maxIndent;
}

void GeneralLogger::info(const QString& msg, const GeneralLogger::LogIndent indent) {
    if (indent > g_maxIndent) return;
    QString indentedMsg = QString("  ").repeated(indent) + msg;
    qCInfo(logMain).noquote() << indentedMsg;
}

void GeneralLogger::warn(const QString& msg, const GeneralLogger::LogIndent indent) {
    if (indent > g_maxIndent) return;

    QString indentedMsg = QString("  ").repeated(indent) + msg;
    qCWarning(logMain).noquote() << indentedMsg;
}

void GeneralLogger::error(const QString& msg, const GeneralLogger::LogIndent indent) {
    QString indentedMsg = QString("  ").repeated(indent) + msg;
    qCCritical(logMain).noquote() << indentedMsg;
}
