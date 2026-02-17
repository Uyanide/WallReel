#include "configmgr.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcessEnvironment>
#include <QStandardPaths>

#include "utils/logger.hpp"
#include "utils/misc.hpp"
using namespace GeneralLogger;

const QString Config::s_DefaultConfigFileName = "config.json";

Config::Config(
    const QString& configDir,
    const QStringList& searchDirs,
    const QString& configPath,
    QObject* parent)
    : QObject(parent), m_configDir(configDir) {
    if (configPath.isEmpty()) {
        info(QString("Configuration directory: %1").arg(configDir));
        _loadConfig(configDir + QDir::separator() + s_DefaultConfigFileName);
    } else {
        _loadConfig(configPath);
    }
    if (!searchDirs.isEmpty()) {
        info(QString("Additional search directories: %1").arg(searchDirs.join(", ")));
        for (const auto& dir : searchDirs) {
            m_wallpaperConfig.dirs.append({dir, false});
        }
    }

    debug("Loading wallpapers ...");
    _loadWallpapers();
}

Config::~Config() {
}

void Config::_loadConfig(const QString& configPath) {
    info(QString("Loading configuration from: %1").arg(configPath));
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        critical(QString("Failed to open config file: %1").arg(configPath));
        return;
    }
    QByteArray configData = configFile.readAll();
    configFile.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(configData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        critical(QString("Invalid JSON format in config file"));
        return;
    }

    const auto jsonObj = jsonDoc.object();

    _loadWallpaperConfig(jsonObj);
    _loadPaletteConfig(jsonObj);
    _loadActionConfig(jsonObj);
    _loadStyleConfig(jsonObj);
    _loadSortConfig(jsonObj);
}

void Config::_loadWallpaperConfig(const QJsonObject& root) {
    if (!root.contains("wallpaper") || !root["wallpaper"].isObject()) {
        return;
    }
    const QJsonObject& config = root["wallpaper"].toObject();

    if (config.contains("paths") && config["paths"].isArray()) {
        for (const auto& item : config["paths"].toArray()) {
            if (item.isString()) {
                m_wallpaperConfig.paths.append(::expandPath(item.toString()));
            }
        }
    }

    if (config.contains("dirs") && config["dirs"].isArray()) {
        for (const auto& item : config["dirs"].toArray()) {
            if (item.isObject()) {
                QJsonObject obj = item.toObject();
                if (obj.contains("path") && obj["path"].isString()) {
                    WallpaperConfigItems::WallpaperDirConfigItem dirConfig;
                    dirConfig.path = ::expandPath(obj["path"].toString());
                    if (obj.contains("recursive") && obj["recursive"].isBool()) {
                        dirConfig.recursive = obj["recursive"].toBool();
                    } else {
                        dirConfig.recursive = false;
                    }
                    m_wallpaperConfig.dirs.append(dirConfig);
                }
            }
        }
    }

    if (config.contains("excludes") && config["excludes"].isArray()) {
        for (const auto& item : config["excludes"].toArray()) {
            if (item.isString()) {
                auto regex = QRegularExpression(item.toString());
                if (!regex.isValid()) {
                    warn(QString("Invalid regular expression in config: %1").arg(item.toString()));
                } else {
                    m_wallpaperConfig.excludes.append(regex);
                }
            }
        }
    }
}

void Config::_loadPaletteConfig(const QJsonObject& root) {
    if (!root.contains("palettes") || !root["palettes"].isArray()) {
        return;
    }
    const QJsonArray& palettes = root["palettes"].toArray();

    for (const auto& palItem : palettes) {
        if (palItem.isObject()) {
            QJsonObject palObj = palItem.toObject();
            PaletteConfigItems::PaletteConfigItem palette;
            if (palObj.contains("name") && palObj["name"].isString()) {
                palette.name = palObj["name"].toString();
            }
            if (palObj.contains("colors") && palObj["colors"].isArray()) {
                for (const auto& colorItem : palObj["colors"].toArray()) {
                    PaletteConfigItems::PaletteColorConfigItem colorConfig;
                    if (colorItem.isObject()) {
                        QJsonObject colorObj = colorItem.toObject();
                        if (colorObj.contains("name") && colorObj["name"].isString()) {
                            colorConfig.name = colorObj["name"].toString();
                        }
                        if (colorObj.contains("value") && colorObj["value"].isString()) {
                            QColor color(colorObj["value"].toString());
                            if (color.isValid()) {
                                colorConfig.value = color;
                            } else {
                                warn(QString("Invalid color string in config: %1").arg(colorObj["value"].toString()));
                            }
                        }
                    } else if (colorItem.isString()) {
                        QColor color(colorItem.toString());
                        if (color.isValid()) {
                            colorConfig.value = color;
                        } else {
                            warn(QString("Invalid color string in config: %1").arg(colorItem.toString()));
                        }
                    }
                    if (colorConfig.value.isValid()) {
                        palette.colors.append(colorConfig);
                    }
                }
            }
            m_paletteConfig.palettes.append(palette);
        }
    }
}

void Config::_loadActionConfig(const QJsonObject& root) {
    if (!root.contains("action") || !root["action"].isObject()) {
        return;
    }
    const QJsonObject& config = root["action"].toObject();

    if (config.contains("previewDebounceTime")) {
        const auto& val = config["previewDebounceTime"];
        if (val.isDouble() && val.toDouble() >= 0) {
            m_actionConfig.previewDebounceTime = val.toInt();
        }
    }
    if (config.contains("printSelected")) {
        const auto& val = config["printSelected"];
        if (val.isBool()) {
            m_actionConfig.printSelected = val.toBool();
        }
    }
    if (config.contains("printPreview")) {
        const auto& val = config["printPreview"];
        if (val.isBool()) {
            m_actionConfig.printPreview = val.toBool();
        }
    }
    if (config.contains("saveState")) {
        const auto& val = config["saveState"];
        if (val.isObject()) {
            QJsonObject obj = val.toObject();
            for (const auto& key : obj.keys()) {
                if (obj[key].isString()) {
                    m_actionConfig.saveState.insert(key, obj[key].toString());
                }
            }
        }
    }
    if (config.contains("onRestore")) {
        const auto& val = config["onRestore"];
        if (val.isString()) {
            m_actionConfig.onRestore = val.toString();
        }
    }
    if (config.contains("onSelected")) {
        const auto& val = config["onSelected"];
        if (val.isString()) {
            m_actionConfig.onSelected = val.toString();
        }
    }
    if (config.contains("onPreview")) {
        const auto& val = config["onPreview"];
        if (val.isString()) {
            m_actionConfig.onPreview = val.toString();
        }
    }
}

void Config::_loadStyleConfig(const QJsonObject& root) {
    if (!root.contains("style") || !root["style"].isObject()) {
        return;
    }
    const QJsonObject& config = root["style"].toObject();

    if (config.contains("image_width")) {
        const auto& val = config["image_width"];
        if (val.isDouble() && val.toDouble() > 0) {
            m_styleConfig.imageWidth = val.toInt();
        }
    }
    if (config.contains("image_height")) {
        const auto& val = config["image_height"];
        if (val.isDouble() && val.toDouble() > 0) {
            m_styleConfig.imageHeight = val.toInt();
        }
    }
    if (config.contains("image_focus_scale")) {
        const auto& val = config["image_focus_scale"];
        if (val.isDouble() && val.toDouble() > 0) {
            m_styleConfig.imageFocusScale = val.toDouble();
        }
    }
    if (config.contains("window_width")) {
        const auto& val = config["window_width"];
        if (val.isDouble() && val.toDouble() > 0) {
            m_styleConfig.windowWidth = val.toInt();
        }
    }
    if (config.contains("window_height")) {
        const auto& val = config["window_height"];
        if (val.isDouble() && val.toDouble() > 0) {
            m_styleConfig.windowHeight = val.toInt();
        }
    }
}

void Config::_loadSortConfig(const QJsonObject& root) {
    if (!root.contains("sort") || !root["sort"].isObject()) {
        return;
    }
    const QJsonObject& config = root["sort"].toObject();

    if (config.contains("type")) {
        const auto& val = config["type"];
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
                warn(QString("Unknown sort type: %1").arg(type));
            }
        }
    }
    if (config.contains("reverse")) {
        const auto& val = config["reverse"];
        if (val.isBool()) {
            m_sortConfig.reverse = val.toBool();
        }
    }
}

void Config::_loadWallpapers() {
    m_wallpapers.clear();

    QSet<QString> paths;

    debug(QString("Loading wallpapers from %1 specified paths...").arg(m_wallpaperConfig.paths.size()));
    for (const QString& path : std::as_const(m_wallpaperConfig.paths)) {
        paths.insert(path);
    }

    debug(QString("Loading wallpapers from %1 specified directories...").arg(m_wallpaperConfig.dirs.size()));
    for (const auto& dirConfig : std::as_const(m_wallpaperConfig.dirs)) {
        if (checkDir(dirConfig.path)) {
            std::function<void(const QDir&)> scanDir;
            scanDir = [&](const QDir& d) {
                QStringList files = d.entryList(QDir::Files | QDir::NoDotAndDotDot);
                for (const QString& file : std::as_const(files)) {
                    QString filePath = d.filePath(file);
                    paths.insert(expandPath(filePath));
                }

                if (dirConfig.recursive) {
                    QStringList subDirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                    debug(QString("Scanning directory '%1' for subdirectories... Found %2").arg(d.absolutePath()).arg(subDirs.size()));
                    for (const QString& subDir : std::as_const(subDirs)) {
                        scanDir(QDir(d.filePath(subDir)));
                    }
                }
            };
            scanDir(QDir(dirConfig.path));
        } else {
            warn(QString("Directory '%1' does not exist").arg(dirConfig.path));
        }
    }

    debug(QString("Excluding %1 specified paths...").arg(m_wallpaperConfig.excludes.size()));
    QStringList toRemove;
    for (const auto& exclude : std::as_const(m_wallpaperConfig.excludes)) {
        for (const QString& path : std::as_const(paths)) {
            if (exclude.match(path).hasMatch()) {
                toRemove.append(path);
                debug(QString("Excluded path '%1' matched by regex '%2'").arg(path).arg(exclude.pattern()));
            }
        }
    }
    for (const auto& path : toRemove) {
        paths.remove(path);
    }

    m_wallpapers.reserve(paths.size());
    for (const QString& path : paths) {
        if (checkImageFile(path)) {
            m_wallpapers.append(path);
        } else {
            warn(QString("File '%1' is not recognized as a valid image file").arg(path));
        }
    }

    info(QString("Found %1 files").arg(paths.size()));
}
