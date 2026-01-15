/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-05 01:34:52
 * @LastEditTime: 2026-01-15 07:18:46
 * @Description: Configuration manager.
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QString>
#include <QStringList>

// Config entries:
//
// wallpaper.paths          array       image paths
// wallpaper.dirs           array       directories to search for images.
//                                      all images in these directories will be added.
//                                      NOT recursive.
// wallpaper.excludes       array       exclude patterns
//
// action.confirm           string      command to execute on confirm
//
// style.aspect_ratio       number      (width / height) of each image
// style.image_width        number      width of each image
// style.image_focus_width  number      width of focused image
// style.window_width       number      fixed window width
// style.window_height      number      fixed window height
// style.no_loading_screen  boolean     disable loading screen and load images while updating UI in batches
//
// sort.type                string      sorting type: "none", "name", "date", "size"
// sort.reverse             boolean     whether to reverse the sorting order

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
        QStringList paths;     // "wallpaper.paths"
        QStringList dirs;      // "wallpaper.dirs"
        QStringList excludes;  // "wallpaper.excludes"
    };

    struct ActionConfigItems {
        QString confirm;  // "action.confirm"
    };

    struct StyleConfigItems {
        double aspectRatio   = 1.6;    // "style.aspect_ratio"
        int imageWidth       = 320;    // "style.image_width"
        int imageFocusWidth  = 480;    // "style.image_focus_width"
        int windowWidth      = 750;    // "style.window_width"
        int windowHeight     = 500;    // "style.window_height"
        bool noLoadingScreen = false;  // "style.no_loading_screen"
    };

    struct SortConfigItems {
        SortType type = SortType::Name;  // "sort.type"
        bool reverse  = false;           // "sort.reverse"
    };

    Config(
        const QString& configDir,  // Fixed, usually "~/.config/wallpaper-carousel"
        const QStringList& searchDirs = {},
        const QString& configPath     = "",  // Override the default config path
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
