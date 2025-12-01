/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-05 10:43:31
 * @LastEditTime: 2025-12-01 01:12:53
 * @Description: A simple thread-safe logger.
 */
#ifndef GENERAL_LOGGER_H
#define GENERAL_LOGGER_H

#include <QLoggingCategory>
#include <QString>
#include <cstdio>

Q_DECLARE_LOGGING_CATEGORY(logMain)

namespace GeneralLogger {

enum LogIndent : qint32 {
    GENERAL = 0,
    STEP    = 1,
    DETAIL  = 2,
};

void info(const QString& msg,
          const LogIndent indent = GENERAL);

void warn(const QString& msg,
          const LogIndent indent = GENERAL);

void error(const QString& msg,
           const LogIndent indent = GENERAL);
}  // namespace GeneralLogger

class Logger {
  public:
    static void init(FILE* stream                       = stderr,
                     GeneralLogger::LogIndent maxIndent = GeneralLogger::DETAIL);

    static void setMaxIndent(GeneralLogger::LogIndent indent);
    static GeneralLogger::LogIndent maxIndent();
};

#endif  // GENERAL_LOGGER_H