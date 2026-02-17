#ifndef WALLREEL_LOGGER_HPP
#define WALLREEL_LOGGER_HPP

#include <QLoggingCategory>
#include <QString>
#include <cstdio>

Q_DECLARE_LOGGING_CATEGORY(logMain)

namespace WallReel::Core {
class Logger {
  public:
    static void debug(const QString& msg);

    static void info(const QString& msg);

    static void warn(const QString& msg);

    static void critical(const QString& msg);

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

}  // namespace WallReel::Core

#endif  // WALLREEL_LOGGER_HPP
