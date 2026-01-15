/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-11-30 20:59:57
 * @LastEditTime: 2026-01-15 03:11:08
 * @Description: THE utils header that every project needs :)
 */

#ifndef UTILS_H
#define UTILS_H

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <utility>

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

inline bool checkFile(const QString& path) {
    QFileInfo checkFile(path);
    return checkFile.exists() && checkFile.isFile() && checkFile.isReadable();
}

inline bool checkDir(const QString& path) {
    QFileInfo checkFile(path);
    return checkFile.exists() && checkFile.isDir() && checkFile.isReadable();
}

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

inline bool checkImageFile(const QString& filePath) {
    static const QStringList validExtensions = {
        ".jpg",
        ".jpeg",
        ".jfif",
        ".png",
        ".bmp",
        ".gif",
        ".webp",
        ".tiff",
        ".avif",
        ".heic",
        ".heif"};

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
    for (const QString& ext : validExtensions) {
        if (filePath.endsWith(ext, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

#endif  // UTILS_H
