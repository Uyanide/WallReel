/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-11-30 20:59:57
 * @LastEditTime: 2026-01-18 06:36:13
 * @Description: THE utils header that every project needs :)
 */

#ifndef UTILS_H
#define UTILS_H

#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <utility>

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
 * @brief Split the file name from a given path.
 *
 * @param path
 * @return QString
 */
static QString splitNameFromPath(const QString& path) {
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

#endif  // UTILS_H
