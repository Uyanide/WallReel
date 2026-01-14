/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-05 01:34:52
 * @LastEditTime: 2025-08-07 22:27:50
 * @Description: Configuration manager.
 */
#include "config.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcessEnvironment>
#include <QStandardPaths>

#include "logger.h"
using namespace GeneralLogger;

static QString expandPath(const QString& path);

const QString Config::s_DefaultConfigFileName = "config.json";

Config::Config(const QString& configDir, const QStringList& searchDirs, QObject* parent)
    : QObject(parent), m_configDir(configDir) {
    info(QString("Loading configuration from: %1").arg(configDir));
    _loadConfig(configDir + QDir::separator() + s_DefaultConfigFileName);

    info(QString("Additional search directories: %1").arg(searchDirs.join(", ")));
    m_wallpaperConfig.dirs.append(searchDirs);

    info("Loading wallpapers ...");
    _loadWallpapers();
}

Config::~Config() {
}

void Config::_loadConfig(const QString& configPath) {
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        error(QString("Failed to open config file: %1").arg(configPath));
        return;
    }
    QByteArray configData = configFile.readAll();
    configFile.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(configData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        error(QString("Invalid JSON format in config file"));
        return;
    }

    const auto jsonObj = jsonDoc.object();

    struct ConfigMapping {
        QString path;
        QString key;
        std::function<void(const QJsonValue&)> parser;
    };

    static const auto parseJsonArray = [](const QJsonValue& val, QStringList& list) {
        if (val.isArray()) {
            for (const auto& item : val.toArray()) {
                if (item.isString()) {
                    list.append(::expandPath(item.toString()));
                }
            }
        }
    };

    std::vector<ConfigMapping>
        mappings = {
            {"wallpaper.paths", "paths", [this](const QJsonValue& val) {
                 parseJsonArray(val, m_wallpaperConfig.paths);
             }},
            {"wallpaper.dirs", "dirs", [this](const QJsonValue& val) {
                 parseJsonArray(val, m_wallpaperConfig.dirs);
             }},
            {"wallpaper.excludes", "excludes", [this](const QJsonValue& val) {
                 parseJsonArray(val, m_wallpaperConfig.excludes);
             }},
            {"action.confirm", "confirm", [this](const QJsonValue& val) {
                 if (val.isString()) {
                     m_actionConfig.confirm = ::expandPath(val.toString());
                     info(QString("Action confirm: %1").arg(m_actionConfig.confirm), GeneralLogger::STEP);
                 }
             }},
            {"style.aspect_ratio", "aspect_ratio", [this](const QJsonValue& val) {
                 if (val.isDouble() && val.toDouble() > 0) {
                     m_styleConfig.aspectRatio = val.toDouble();
                     info(QString("Aspect ratio: %1").arg(m_styleConfig.aspectRatio), GeneralLogger::STEP);
                 }
             }},
            {"style.image_width", "image_width", [this](const QJsonValue& val) {
                 if (val.isDouble() && val.toDouble() > 0) {
                     m_styleConfig.imageWidth = val.toInt();
                     info(QString("Image width: %1").arg(m_styleConfig.imageWidth), GeneralLogger::STEP);
                 }
             }},
            {"style.image_focus_width", "image_focus_width", [this](const QJsonValue& val) {
                 if (val.isDouble() && val.toDouble() > 0) {
                     m_styleConfig.imageFocusWidth = val.toInt();
                     info(QString("Image focus width: %1").arg(m_styleConfig.imageFocusWidth), GeneralLogger::STEP);
                 }
             }},
            {"style.window_width", "window_width", [this](const QJsonValue& val) {
                 if (val.isDouble() && val.toDouble() > 0) {
                     m_styleConfig.windowWidth = val.toInt();
                     info(QString("Window width: %1").arg(m_styleConfig.windowWidth), GeneralLogger::STEP);
                 }
             }},
            {"style.window_height", "window_height", [this](const QJsonValue& val) {
                 if (val.isDouble() && val.toDouble() > 0) {
                     m_styleConfig.windowHeight = val.toInt();
                     info(QString("Window height: %1").arg(m_styleConfig.windowHeight), GeneralLogger::STEP);
                 }
             }},
            {"style.no_loading_screen", "no_loading_screen", [this](const QJsonValue& val) {
                 if (val.isBool()) {
                     m_styleConfig.noLoadingScreen = val.toBool();
                     info(QString("No loading screen: %1").arg(m_styleConfig.noLoadingScreen), GeneralLogger::STEP);
                 }
             }},
            {"sort.type", "type", [this](const QJsonValue& val) {
                 if (val.isString()) {
                     QString type = val.toString().toLower();
                     if (type == "none") {
                         m_sortConfig.type = SortType::None;
                     } else if (type == "name") {
                         m_sortConfig.type = SortType::Name;
                     } else if (type == "date") {
                         m_sortConfig.type = SortType::Date;
                     } else if (type == "size") {
                         m_sortConfig.type = SortType::Size;
                     } else {
                         warn(QString("Unknown sort type: %1").arg(type), GeneralLogger::STEP);
                     }
                 }
                 info(QString("Sort type: %1").arg(static_cast<int>(m_sortConfig.type)), GeneralLogger::STEP);
             }},
            {"sort.reverse", "reverse", [this](const QJsonValue& val) {
                 if (val.isBool()) {
                     m_sortConfig.reverse = val.toBool();
                     info(QString("Sort reverse: %1").arg(m_sortConfig.reverse), GeneralLogger::STEP);
                 }
             }},
        };

    // 统一解析
    for (const auto& mapping : mappings) {
        ([&mapping, &jsonObj]() {
            auto pathParts = mapping.path.split('.');

            QJsonObject currentObj = jsonObj;
            QJsonValue targetValue;

            for (int i = 0; i < pathParts.size() - 1; ++i) {
                if (currentObj.contains(pathParts[i]) && currentObj[pathParts[i]].isObject()) {
                    currentObj = currentObj[pathParts[i]].toObject();
                } else {
                    warn(QString("Path '%1' not found").arg(pathParts.mid(0, i + 1).join('.')));
                    return;
                }
            }

            // 获取目标值
            const QString& finalKey = pathParts.last();
            if (currentObj.contains(finalKey)) {
                mapping.parser(currentObj[finalKey]);
            } else {
                warn(QString("Key '%1' not found in '%2'").arg(finalKey).arg(mapping.path));
            }
        })();
    }
}

void Config::_loadWallpapers() {
    m_wallpapers.clear();

    QSet<QString> paths;

    info(QString("Loading wallpapers from %1 specified paths").arg(m_wallpaperConfig.paths.size()), LogIndent::STEP);
    for (const QString& path : m_wallpaperConfig.paths) {
        paths.insert(path);
    }

    info(QString("Loading wallpapers from %1 specified directories").arg(m_wallpaperConfig.dirs.size()), LogIndent::STEP);
    for (const QString& dirPath : m_wallpaperConfig.dirs) {
        QDir dir(dirPath);
        if (dir.exists()) {
            QStringList files = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);
            for (const QString& file : files) {
                QString filePath = dir.filePath(file);
                paths.insert(filePath);
            }
        } else {
            warn(QString("Directory '%1' does not exist").arg(dirPath));
        }
    }

    info(QString("Excluding %1 specified paths").arg(m_wallpaperConfig.excludes.size()), LogIndent::STEP);
    for (const QString& exclude : m_wallpaperConfig.excludes) {
        paths.remove(exclude);
    }

    m_wallpapers.reserve(paths.size());
    for (const QString& path : paths) {
        if (isValidImageFile(path)) {
            m_wallpapers.append(path);
        }
    }

    info(QString("Found %1 wallpapers").arg(paths.size()));
}

bool Config::isValidImageFile(const QString& filePath) {
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
        warn(QString("File does not exist: %1").arg(filePath));
        return false;
    }
    // check if normal file
    QFileInfo fileInfo(filePath);
    if (!(fileInfo.isFile() || fileInfo.isSymbolicLink()) || !fileInfo.isReadable()) {
        warn(QString("Invalid file: %1").arg(filePath));
        return false;
    }
    // check if valid extension
    for (const QString& ext : validExtensions) {
        if (filePath.endsWith(ext, Qt::CaseInsensitive)) {
            return true;
        }
    }
    warn(QString("Unsupported file type: %1").arg(filePath));
    return false;
}

static QString expandPath(const QString& path) {
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
