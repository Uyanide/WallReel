#ifndef WALLREEL_CONFIGMGR_HPP
#define WALLREEL_CONFIGMGR_HPP

#include <qregularexpression.h>

#include <QColor>
#include <QObject>
#include <QRegularExpression>
#include <QSize>
#include <QString>
#include <QStringList>

// Config entries:
//
// wallpaper.paths              array   []      List of paths to images.
// wallpaper.dirs               array   []      Directories to search for images.
// wallpaper.dirs[].path        string  ""      Path to the directory.
// wallpaper.dirs[].recursive   boolean false   Whether to search the directory recursively.
// wallpaper.excludes           array   []      Exclude patterns (regex)
//
// palettes                     array   []
// palettes[].name              string  ""      Name of the palette
// palettes[].colors            array   []      List of colors in the palette
// palettes[].colors[].name     string  ""      Name of the color
// palettes[].colors[].value    string  ""      Color value in hex format, e.g. "#ff0000" for red
//
// action.previewDebounceTime   number  300     Minimum debounce time for preview action in milliseconds
// action.printSelected         boolean false   Whether to print the selected wallpaper path to stdout on confirm
// action.printPreview          boolean false   Whether to print the previewed wallpaper path to stdout on preview
// action.saveState             object  {}      Key-value pairs to save the state, useful for restore command
// action.onRestore             string  ""      Command to execute on restore ({{ key }} -> value in saveState)
// action.onSelected            string  ""      Command to execute on confirmation ({{ path }} -> full path)
// action.onPreview             string  ""      Command to execute on preview ({{ path }} -> full path)
//
// style.image_width            number  320     Width of each image
// style.image_height           number  200     Height of each image
// style.image_focus_scale      number  1.5     Scale of the focused image (relative to unfocused image)
// style.window_width           number  750     Initial window width
// style.window_height          number  500     Initial window height
//
// sort.type                    string  "name"  Sorting type: "none", "name", "date", "size"
// sort.reverse                 boolean false   Whether to reverse the sorting order
//                                              Normal order: name: lexicographical, e.g. "a.jpg" before "b.jpg"
//                                                            date: older before newer
//                                                            size: smaller before larger

class Config : public QObject {
    Q_OBJECT

    Q_PROPERTY(int imageWidth READ getImageWidth CONSTANT)
    Q_PROPERTY(int imageHeight READ getImageHeight CONSTANT)
    Q_PROPERTY(double imageFocusScale READ getImageFocusScale CONSTANT)
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
        struct WallpaperDirConfigItem {
            QString path;
            bool recursive;
        };

        QStringList paths;
        QList<WallpaperDirConfigItem> dirs;
        QList<QRegularExpression> excludes;
    };

    struct PaletteConfigItems {
        struct PaletteColorConfigItem {
            QString name;
            QColor value;
        };

        struct PaletteConfigItem {
            QString name;
            QList<PaletteColorConfigItem> colors;
        };

        QList<PaletteConfigItem> palettes;
    };

    struct ActionConfigItems {

        QHash<QString, QString> saveState;
        QString onSelected;
        QString onPreview;
        QString onRestore;
        int previewDebounceTime = 300;  // milliseconds
        bool printSelected      = false;
        bool printPreview       = false;
    };

    struct StyleConfigItems {
        double imageFocusScale = 1.5;
        int imageWidth         = 320;
        int imageHeight        = 200;
        int windowWidth        = 750;
        int windowHeight       = 500;
    };

    struct SortConfigItems {
        SortType type = SortType::Name;
        bool reverse  = false;
    };

    Config(
        const QString& configDir,
        const QStringList& searchDirs = {},
        const QString& configPath     = "",  // Override the default config path
        QObject* parent               = nullptr);

    ~Config();

    const QStringList& getWallpapers() const { return m_wallpapers; }

    qint64 getWallpaperCount() const { return m_wallpapers.size(); }

    const WallpaperConfigItems& getWallpaperConfig() const { return m_wallpaperConfig; }

    const PaletteConfigItems& getPaletteConfig() const { return m_paletteConfig; }

    const ActionConfigItems& getActionConfig() const { return m_actionConfig; }

    const StyleConfigItems& getStyleConfig() const { return m_styleConfig; }

    const SortConfigItems& getSortConfig() const { return m_sortConfig; }

    int getImageWidth() const { return m_styleConfig.imageWidth; }

    int getImageHeight() const { return m_styleConfig.imageHeight; }

    double getImageFocusScale() const { return m_styleConfig.imageFocusScale; }

    int getWindowWidth() const { return m_styleConfig.windowWidth; }

    int getWindowHeight() const { return m_styleConfig.windowHeight; }

    QSize getFocusImageSize() const {
        return QSize{m_styleConfig.imageWidth, m_styleConfig.imageHeight} * m_styleConfig.imageFocusScale;
    }

    static const QString s_DefaultConfigFileName;
    const QString m_configDir;

  private:
    void _loadConfig(const QString& configPath);
    void _loadWallpapers();
    void _loadWallpaperConfig(const QJsonObject& config);
    void _loadPaletteConfig(const QJsonObject& config);
    void _loadActionConfig(const QJsonObject& config);
    void _loadStyleConfig(const QJsonObject& config);
    void _loadSortConfig(const QJsonObject& config);

  private:
    WallpaperConfigItems m_wallpaperConfig;
    PaletteConfigItems m_paletteConfig;
    ActionConfigItems m_actionConfig;
    StyleConfigItems m_styleConfig;
    SortConfigItems m_sortConfig;

    QStringList m_wallpapers;
};

#endif  // WALLREEL_CONFIGMGR_HPP
