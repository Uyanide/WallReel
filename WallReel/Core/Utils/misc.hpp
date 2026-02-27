#ifndef WALLREEL_MISC_HPP
#define WALLREEL_MISC_HPP

#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <utility>

#include "version.h"

namespace WallReel::Core::Utils {

/**
 * @brief Defer execution of a callable until the end of the current scope.
 *
 * @tparam Callable
 */
template <typename Callable>
class Defer {
    Callable m_func;

  public:
    explicit Defer(Callable&& func)
        : m_func(std::forward<Callable>(func)) {}

    Defer()             = delete;
    Defer(const Defer&) = delete;

    ~Defer() {
        m_func();
    }
};

/**
 * @brief Check if a file exists, is a regular file, and is readable.
 *
 * @param path
 */
inline bool checkFile(const QString& path) {
    QFileInfo checkFile(path);
    // According to Qt docs, "exists() returns true if the symlink points to an existing target, otherwise it returns false."
    // So no need to separately check for isSymbolicLink() or isSymLink().
    return checkFile.exists() && checkFile.isFile() && checkFile.isReadable();
}

/**
 * @brief Check if a directory exists, is a directory, and is readable.
 *
 * @param path
 */
inline bool checkDir(const QString& path) {
    QFileInfo checkFile(path);
    return checkFile.exists() && checkFile.isDir() && checkFile.isReadable();
}

/**
 * @brief Expand environment variables and ~ in a given path.
 *
 * @param path Input path
 * @return QString Expanded path
 */
inline QString expandPath(const QString& path) {
    QString expandedPath = path;

    if (expandedPath.startsWith("~/")) {
        expandedPath.replace(0, 1, QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    } else if (expandedPath == "~") {
        expandedPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QRegularExpression envVarRegex(R"(\$([A-Za-z_][A-Za-z0-9_]*))");
    QRegularExpressionMatchIterator i = envVarRegex.globalMatch(expandedPath);

    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString varName               = match.captured(1);
        QString varValue              = env.value(varName);
        if (!varValue.isEmpty()) {
            expandedPath.replace(match.captured(0), varValue);
        }
    }

    return QDir::cleanPath(expandedPath);
}

/**
 * @brief Convert the given path to an absolute path. If it's already absolute, return as is.
 *        If it's relative, make it absolute based on the current working directory.
 *
 * @param path Input path
 * @return QString Absolute path
 *
 * @note No guarantee that the returned path actually exists or is valid.
 * @note Symbolic links are not resolved.
 * @note The returned path is cleaned using QDir::cleanPath()
 */
inline QString ensureAbsolutePath(const QString& path) {
    if (QDir::isAbsolutePath(path)) {
        return path;
    } else {
        return QDir::cleanPath(QDir::current().filePath(path));
    }
}

/**
 * @brief Split the file name from a given path.
 *
 * @param path
 * @return QString
 */
inline QString splitNameFromPath(const QString& path) {
    QFileInfo fileInfo(path);
    return fileInfo.fileName();
}

/**
 * @brief In addition to checking if the file exists and is readable,
 *        also checks if the file has a valid image extension.
 *
 * @param filePath
 * @return true
 * @return false
 */
inline bool checkImageFile(const QString& filePath) {
    // check if exist
    if (!QFile::exists(filePath)) {
        return false;
    }
    // check if normal file
    QFileInfo fileInfo(filePath);
    if (!(fileInfo.isFile() || fileInfo.isSymbolicLink()) || !fileInfo.isReadable()) {
        return false;
    }
    // check if valid extension
    static const QList<QByteArray> formats = QImageReader::supportedImageFormats();
    QString ext                            = QFileInfo(filePath).suffix().toLower();
    return formats.contains(ext.toUtf8());
}

/**
 * @brief Get the configuration directory for the application, and create it if it doesn't exist.
 *
 * @return QDir The configuration directory, typically ~/.config/AppName
 */
inline QDir getConfigDir() {
    // This will be ~/.config/AppName, where AppName is the name of executable target in CMakeLists.txt
    auto configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (configDir.isEmpty()) {
        configDir = QDir::homePath() + QDir::separator() + ".config" + QDir::separator() + APP_NAME;
    }
    QDir().mkpath(configDir);
    return configDir;
}

/**
 * @brief Get the cache directory for the application, and create it if it doesn't exist.
 *
 * @return QDir The cache directory, typically ~/.cache/AppName
 */
inline QDir getCacheDir() {
    // This will be ~/.cache/AppName, where AppName is the name of executable target in CMakeLists.txt
    auto cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (cacheDir.isEmpty()) {
        cacheDir = QDir::homePath() + QDir::separator() + ".cache" + QDir::separator() + APP_NAME;
    }
    QDir().mkpath(cacheDir);
    return QDir(cacheDir);
}

}  // namespace WallReel::Core::Utils

#endif  // WALLREEL_MISC_HPP
