/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-05 01:34:52
 * @LastEditTime: 2026-01-15 03:40:25
 * @Description: Configuration manager.
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QString>
#include <QStringList>

class Config : public QObject {
    Q_OBJECT

  public:
    enum class SortType : int {
        None = 0,  // "none"
        Name,      // "name"
        Date,      // "date"
        Size,      // "size"
    };

    struct WallpaperConfigItems {
        QStringList paths;
        QStringList dirs;
        QStringList excludes;
    };

    struct ActionConfigItems {
        QString confirm;
    };

    struct StyleConfigItems {
        double aspectRatio   = 1.6;
        int imageWidth       = 320;
        int imageFocusWidth  = 480;
        int windowWidth      = 750;
        int windowHeight     = 500;
        bool noLoadingScreen = false;
    };

    struct SortConfigItems {
        SortType type = SortType::Name;
        bool reverse  = false;
    };

    Config(
        const QString& configDir,
        const QStringList& searchDirs = {},
        const QString& configPath     = "",
        QObject* parent               = nullptr);

    ~Config();

    [[nodiscard]] const QStringList& getWallpapers() const { return m_wallpapers; }

    [[nodiscard]] qint64 getWallpaperCount() const { return m_wallpapers.size(); }

    [[nodiscard]] const WallpaperConfigItems& getWallpaperConfig() const { return m_wallpaperConfig; }

    [[nodiscard]] const ActionConfigItems& getActionConfig() const { return m_actionConfig; }

    [[nodiscard]] const StyleConfigItems& getStyleConfig() const { return m_styleConfig; }

    [[nodiscard]] const SortConfigItems& getSortConfig() const { return m_sortConfig; }

    static const QString s_DefaultConfigFileName;
    const QString m_configDir;

  private:
    void _loadConfig(const QString& configPath);
    void _loadWallpapers();

  private:
    WallpaperConfigItems m_wallpaperConfig;
    ActionConfigItems m_actionConfig;
    StyleConfigItems m_styleConfig;
    SortConfigItems m_sortConfig;

    QStringList m_wallpapers;
};

#endif  // CONFIG_H
