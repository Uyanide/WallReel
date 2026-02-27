#ifndef WALLREEL_LOGGER_HPP
#define WALLREEL_LOGGER_HPP

#include <qcontainerfwd.h>

#include <QLoggingCategory>
#include <QString>
#include <cstdio>

Q_DECLARE_LOGGING_CATEGORY(logMain)

namespace WallReel::Core {

class Logger {
  public:
    static void debug(const QString& sender, const QString& msg);

    static void info(const QString& sender, const QString& msg);

    static void warn(const QString& sender, const QString& msg);

    static void critical(const QString& sender, const QString& msg);

    static void fatal(const QString& sender, const QString& msg);

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

#define WALLREEL_DECLARE_SENDER(name)              \
    namespace {                                    \
    constexpr const char* _wallreel_sender = name; \
    }

#define WR_DEBUG(msg) WallReel::Core::Logger::debug(_wallreel_sender, msg)
#define WR_INFO(msg) WallReel::Core::Logger::info(_wallreel_sender, msg)
#define WR_WARN(msg) WallReel::Core::Logger::warn(_wallreel_sender, msg)
#define WR_CRITICAL(msg) WallReel::Core::Logger::critical(_wallreel_sender, msg)
#define WR_FATAL(msg) WallReel::Core::Logger::fatal(_wallreel_sender, msg)

#endif  // WALLREEL_LOGGER_HPP
