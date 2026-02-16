#ifndef WALLREEL_LOGGER_HPP
#define WALLREEL_LOGGER_HPP

#include <QLoggingCategory>
#include <QString>
#include <cstdio>

Q_DECLARE_LOGGING_CATEGORY(logMain)

namespace GeneralLogger {

void debug(const QString& msg);

void info(const QString& msg);

void warn(const QString& msg);

void critical(const QString& msg);

}  // namespace GeneralLogger

class Logger {
  public:
    /**
     * @brief Initialize the logger and set the output stream.
     *
     * @param stream
     */
    static void init(FILE* stream = stderr);

    /**
     * @brief Set the log level.
     *
     * @param level
     */
    static void setLogLevel(QtMsgType level);

    /**
     * @brief Suppress all log output.
     *
     */
    static void quiet();
};

#endif  // WALLREEL_LOGGER_HPP
