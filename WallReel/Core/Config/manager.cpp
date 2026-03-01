#include "manager.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QTimer>

#include "Utils/misc.hpp"
#include "logger.hpp"

WALLREEL_DECLARE_SENDER("ConfigManager")

namespace WallReel::Core::Config {

const QString CacheConfigItems::defaultSortType        = "Date";
const QString CacheConfigItems::defaultSortDescending  = "true";
const QString CacheConfigItems::defaultSelectedPalette = "";

Manager::Manager(
    const QDir& configDir,
    const QDir& picturesDir,
    const QStringList& searchDirs,
    const QString& configPath,
    QObject* parent)
    : QObject(parent), m_configDir(configDir) {
    connect(this, &Manager::stateCaptured, this, [this]() {
        m_stateCaptured = true;
        WR_INFO("State capture completed");
    });

    // Load configPath if not empty, otherwise load from default location (configDir + s_DefaultConfigFileName)
    if (configPath.isEmpty()) {
        WR_INFO(QString("Configuration directory: %1").arg(m_configDir.absolutePath()));
        _loadConfig(m_configDir.absolutePath() + QDir::separator() + s_DefaultConfigFileName);
    } else {
        _loadConfig(configPath);
    }
    // Append additional search directories to the config
    if (!searchDirs.isEmpty()) {
        WR_INFO(QString("Additional search directories: %1").arg(searchDirs.join(", ")));
        for (const auto& dir : searchDirs) {
            m_wallpaperConfig.dirs.append({dir, false});
        }
    }
    // Add Pictures directory as default search directory if no dirs or paths are specified above
    if (m_wallpaperConfig.dirs.isEmpty() && m_wallpaperConfig.paths.isEmpty()) {
        QString picturesPath = picturesDir.absolutePath();
        WR_INFO(QString("No search directories specified, using Pictures directory: %1").arg(picturesPath));
        m_wallpaperConfig.dirs.append({picturesPath, true});
    }

    WR_DEBUG("Loading wallpapers ...");
    _loadWallpapers();
}

Manager::~Manager() {
}

void Manager::_loadConfig(const QString& configPath) {
    WR_INFO(QString("Loading configuration from: %1").arg(configPath));
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        WR_CRITICAL(QString("Failed to open config file: %1").arg(configPath));
        return;
    }
    QByteArray configData = configFile.readAll();
    configFile.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(configData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        WR_CRITICAL(QString("Invalid JSON format in config file"));
        return;
    }

    const auto jsonObj = jsonDoc.object();

    _loadWallpaperConfig(jsonObj);
    _loadThemeConfig(jsonObj);
    _loadActionConfig(jsonObj);
    _loadStyleConfig(jsonObj);
    _loadCacheConfig(jsonObj);
}

void Manager::_loadWallpaperConfig(const QJsonObject& root) {
    if (!root.contains("wallpaper") || !root["wallpaper"].isObject()) {
        return;
    }
    const QJsonObject& config = root["wallpaper"].toObject();

    if (config.contains("paths") && config["paths"].isArray()) {
        for (const auto& item : config["paths"].toArray()) {
            if (item.isString()) {
                m_wallpaperConfig.paths.append(Utils::ensureAbsolutePath(Utils::expandPath(item.toString())));
            }
        }
    }

    if (config.contains("dirs") && config["dirs"].isArray()) {
        for (const auto& item : config["dirs"].toArray()) {
            if (item.isObject()) {
                QJsonObject obj = item.toObject();
                if (obj.contains("path") && obj["path"].isString()) {
                    WallpaperConfigItems::WallpaperDirConfigItem dirConfig;
                    dirConfig.path = Utils::ensureAbsolutePath(Utils::expandPath(obj["path"].toString()));
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
                    WR_WARN(QString("Invalid regular expression in config: %1").arg(item.toString()));
                } else {
                    m_wallpaperConfig.excludes.append(regex);
                }
            }
        }
    }
}

void Manager::_loadThemeConfig(const QJsonObject& root) {
    if (!root.contains("theme") || !root["theme"].isObject()) {
        return;
    }
    const QJsonObject& theme = root["theme"].toObject();

    if (!theme.contains("palettes") || !theme["palettes"].isArray()) {
        return;
    }
    const QJsonArray& palettes = theme["palettes"].toArray();

    for (const auto& palItem : palettes) {
        if (palItem.isObject()) {
            QJsonObject palObj = palItem.toObject();
            ThemeConfigItems::PaletteConfigItem palette;
            if (palObj.contains("name") && palObj["name"].isString()) {
                palette.name = palObj["name"].toString();
            }
            if (palObj.contains("colors") && palObj["colors"].isArray()) {
                for (const auto& colorItem : palObj["colors"].toArray()) {
                    ThemeConfigItems::PaletteColorConfigItem colorConfig;
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
                                WR_WARN(QString("Invalid color string in config: %1").arg(colorObj["value"].toString()));
                            }
                        }
                    } else if (colorItem.isString()) {
                        QColor color(colorItem.toString());
                        if (color.isValid()) {
                            colorConfig.value = color;
                        } else {
                            WR_WARN(QString("Invalid color string in config: %1").arg(colorItem.toString()));
                        }
                    }
                    if (colorConfig.value.isValid()) {
                        palette.colors.append(colorConfig);
                    }
                }
            }
            m_themeConfig.palettes.append(palette);
        }
    }
}

void Manager::_loadActionConfig(const QJsonObject& root) {
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
    if (config.contains("saveState") && config["saveState"].isArray()) {
        const QJsonArray& arr = config["saveState"].toArray();
        for (const auto& item : arr) {
            if (item.isObject()) {
                QJsonObject obj = item.toObject();
                ActionConfigItems::SaveStateItem sItem;
                if (obj.contains("key") && obj["key"].isString()) {
                    sItem.key = obj["key"].toString();
                }
                if (obj.contains("fallback") && obj["fallback"].isString()) {
                    sItem.defaultVal = obj["fallback"].toString();
                }
                if (obj.contains("command") && obj["command"].isString()) {
                    sItem.command = obj["command"].toString();
                }
                if (obj.contains("timeout") && obj["timeout"].isDouble()) {
                    sItem.timeout = obj["timeout"].toInt();
                }
                if (!sItem.key.isEmpty()) {
                    m_actionConfig.saveStateConfig.append(sItem);
                    m_actionConfig.savedState.insert(sItem.key, sItem.defaultVal);
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
    if (config.contains("quitOnSelected")) {
        const auto& val = config["quitOnSelected"];
        if (val.isBool()) {
            m_actionConfig.quitOnSelected = val.toBool();
        }
    }
    if (config.contains("restoreOnClose")) {
        const auto& val = config["restoreOnClose"];
        if (val.isBool()) {
            m_actionConfig.restoreOnClose = val.toBool();
        }
    }
}

void Manager::_loadStyleConfig(const QJsonObject& root) {
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

void Manager::_loadCacheConfig(const QJsonObject& root) {
    if (!root.contains("cache") || !root["cache"].isObject()) {
        return;
    }
    const QJsonObject& config = root["cache"].toObject();

    if (config.contains("saveSortMethod")) {
        const auto& val = config["saveSortMethod"];
        if (val.isBool()) {
            m_cacheConfig.saveSortMethod = val.toBool();
        }
    }
    if (config.contains("savePalette")) {
        const auto& val = config["savePalette"];
        if (val.isBool()) {
            m_cacheConfig.savePalette = val.toBool();
        }
    }
}

void Manager::_loadWallpapers() {
    m_wallpapers.clear();

    // Add paths first using a set to avoid duplicates

    QSet<QString> paths;

    WR_DEBUG(QString("Loading wallpapers from %1 specified paths...").arg(m_wallpaperConfig.paths.size()));
    for (const QString& path : std::as_const(m_wallpaperConfig.paths)) {
        paths.insert(path);
    }

    WR_DEBUG(QString("Loading wallpapers from %1 specified directories...").arg(m_wallpaperConfig.dirs.size()));
    for (const auto& dirConfig : std::as_const(m_wallpaperConfig.dirs)) {
        if (Utils::checkDir(dirConfig.path)) {
            std::function<void(const QDir&)> scanDir;
            scanDir = [&](const QDir& d) {
                QStringList files = d.entryList(QDir::Files | QDir::NoDotAndDotDot);
                for (const QString& file : std::as_const(files)) {
                    QString filePath = d.filePath(file);
                    paths.insert(Utils::expandPath(filePath));
                }

                if (dirConfig.recursive) {
                    QStringList subDirs = d.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                    WR_DEBUG(QString("Scanning directory '%1' for subdirectories... Found %2").arg(d.absolutePath()).arg(subDirs.size()));
                    for (const QString& subDir : std::as_const(subDirs)) {
                        scanDir(QDir(d.filePath(subDir)));
                    }
                }
            };
            scanDir(QDir(dirConfig.path));
        } else {
            WR_WARN(QString("Directory '%1' does not exist").arg(dirConfig.path));
        }
    }

    // Exclude paths that match any of the exclude regexes

    WR_DEBUG(QString("Excluding %1 specified paths...").arg(m_wallpaperConfig.excludes.size()));
    QStringList toRemove;
    for (const auto& exclude : std::as_const(m_wallpaperConfig.excludes)) {
        for (const QString& path : std::as_const(paths)) {
            if (exclude.match(path).hasMatch()) {
                toRemove.append(path);
                WR_DEBUG(QString("Excluded path '%1' matched by regex '%2'").arg(path).arg(exclude.pattern()));
            }
        }
    }
    for (const auto& path : toRemove) {
        paths.remove(path);
    }

    m_wallpapers.reserve(paths.size());
    for (const QString& path : paths) {
        if (Utils::checkImageFile(path)) {
            m_wallpapers.append(path);
        } else {
            WR_WARN(QString("File '%1' is not recognized as a valid image file").arg(path));
        }
    }

    WR_INFO(QString("Found %1 images").arg(paths.size()));
}

void Manager::captureState() {
    if (m_stateCaptured) {
        WR_DEBUG("State already captured, skipping capture");
        emit stateCaptured();
    }

    if (m_pendingCaptures > 0) {
        WR_WARN("State capture already in progress, ignoring new capture request");
        return;
    }

    m_pendingCaptures = 0;

    const auto& items = m_actionConfig.saveStateConfig;
    if (items.isEmpty()) {
        emit stateCaptured();
        return;
    }

    for (const auto& item : items) {
        if (!item.command.isEmpty()) {
            m_pendingCaptures++;
        }
    }

    if (m_pendingCaptures == 0) {
        emit stateCaptured();
        return;
    }

    for (const auto& item : items) {
        if (item.command.isEmpty()) continue;

        QProcess* process = new QProcess(this);
        QTimer* timer     = nullptr;  // Remains nullptr if no timeout is set for this item
        if (item.timeout > 0) {
            timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setInterval(item.timeout > 0 ? item.timeout : std::numeric_limits<int>::max());
        }

        QString key        = item.key;
        QString defaultVal = item.defaultVal;

        auto onFinished = [this, process, timer, key, defaultVal](const QString& output, bool success) {
            if (timer) {
                timer->stop();
                timer->deleteLater();
            }

            process->disconnect();

            QString result = success ? output : defaultVal;
            WR_DEBUG(QString("Capture result for key '%1': %2 (success: %3)").arg(key).arg(result).arg(success));
            if (result.isEmpty()) result = defaultVal;

            _onCaptureResult(key, result);
            process->deleteLater();
        };

        if (timer) {
            // Timeout handler
            connect(timer, &QTimer::timeout, this, [process, onFinished, key]() {
                WR_WARN(QString("Timeout capturing state for key '%1'").arg(key));
                if (process->state() != QProcess::NotRunning) {
                    process->kill();
                } else {
                    onFinished(QString(), false);
                }
            });
        }

        // Finished handler
        connect(process, &QProcess::finished, this, [process, onFinished](int exitCode, QProcess::ExitStatus exitStatus) {
            bool success = (exitStatus == QProcess::NormalExit && exitCode == 0);
            QString output;
            if (success) {
                output = QString::fromUtf8(process->readAllStandardOutput()).trimmed();
            }
            onFinished(output, success);
        });

        // Error handler
        connect(process, &QProcess::errorOccurred, this, [process, onFinished, key](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
                WR_WARN(QString("Failed to start state command for key '%1'").arg(key));
                onFinished(QString(), false);
            }
        });

        if (timer) {
            timer->start();
        }
        process->start("sh", QStringList() << "-c" << item.command);
    }
}

void Manager::_onCaptureResult(const QString& key, const QString& value) {
    // This is all in main thread, so no lock needed
    m_actionConfig.savedState[key] = value;
    m_pendingCaptures--;
    if (m_pendingCaptures == 0) {
        emit stateCaptured();
    }
}

}  // namespace WallReel::Core::Config
