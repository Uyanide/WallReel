#ifndef WALLREEL_CONFIGMGR_HPP
#define WALLREEL_CONFIGMGR_HPP

#include <QObject>
#include <QQmlEngine>
#include <QSize>
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
//
// sort.type                string      sorting type: "none", "name", "date", "size"
// sort.reverse             boolean     whether to reverse the sorting order

class Config : public QObject {
    Q_OBJECT

    Q_PROPERTY(double aspectRatio READ getAspectRatio CONSTANT)
    Q_PROPERTY(int imageWidth READ getImageWidth CONSTANT)
    Q_PROPERTY(int imageFocusWidth READ getImageFocusWidth CONSTANT)
    Q_PROPERTY(int windowWidth READ getWindowWidth CONSTANT)
    Q_PROPERTY(int windowHeight READ getWindowHeight CONSTANT)

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
        double aspectRatio  = 1.6;  // "style.aspect_ratio"
        int imageWidth      = 320;  // "style.image_width"
        int imageFocusWidth = 480;  // "style.image_focus_width"
        int windowWidth     = 750;  // "style.window_width"
        int windowHeight    = 500;  // "style.window_height"
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

    const QStringList& getWallpapers() const { return m_wallpapers; }

    qint64 getWallpaperCount() const { return m_wallpapers.size(); }

    const WallpaperConfigItems& getWallpaperConfig() const { return m_wallpaperConfig; }

    const ActionConfigItems& getActionConfig() const { return m_actionConfig; }

    const StyleConfigItems& getStyleConfig() const { return m_styleConfig; }

    const SortConfigItems& getSortConfig() const { return m_sortConfig; }

    double getAspectRatio() const { return m_styleConfig.aspectRatio; }

    int getImageWidth() const { return m_styleConfig.imageWidth; }

    int getImageFocusWidth() const { return m_styleConfig.imageFocusWidth; }

    int getWindowWidth() const { return m_styleConfig.windowWidth; }

    int getWindowHeight() const { return m_styleConfig.windowHeight; }

    QSize getFocusImageSize() const {
        int width  = m_styleConfig.imageFocusWidth;
        int height = static_cast<int>(width / m_styleConfig.aspectRatio);
        return {width, height};
    }

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

#endif  // WALLREEL_CONFIGMGR_HPP
