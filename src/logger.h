/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-05 10:43:31
 * @LastEditTime: 2026-01-15 00:57:58
 * @Description: A simple thread-safe logger.
 */
#ifndef GENERAL_LOGGER_H
#define GENERAL_LOGGER_H

#include <QLoggingCategory>
#include <QString>
#include <cstdio>

Q_DECLARE_LOGGING_CATEGORY(logMain)

namespace GeneralLogger {

void debug(const QString& msg);

void info(const QString& msg);

void warn(const QString& msg);

void critical(const QString& msg);

void fatal(const QString& msg);

}  // namespace GeneralLogger

class Logger {
  public:
    static void init(FILE* stream = stderr);

    static void setLogLevel(QtMsgType level);

    static void quiet();
};

#endif  // GENERAL_LOGGER_H
