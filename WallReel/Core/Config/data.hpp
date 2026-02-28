#ifndef WALLREEL_CONFIG_DATA_HPP
#define WALLREEL_CONFIG_DATA_HPP

#include <qtmetamacros.h>

#include <QColor>
#include <QHash>
#include <QList>
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
// theme.defaultPalette                 string  ""      Name of the default palette to use
// theme.palettes                       array   []
// theme.palettes[].name                string  ""      Name of the palette
// theme.palettes[].colors              array   []      List of colors in the palette
// theme.palettes[].colors[].name       string  ""      Name of the color
// theme.palettes[].colors[].value      string  ""      Color value in hex format, e.g. "#ff0000" for red
//
// action.previewDebounceTime   number  300     Debounce time for preview action in milliseconds
// action.printSelected         boolean true    Whether to print the selected wallpaper path to stdout on confirm
// action.printPreview          boolean false   Whether to print the previewed wallpaper path to stdout on preview
// action.onSelected            string  ""      Command to execute on confirmation
// action.onPreview             string  ""      Command to execute on preview
// action.saveState             array   []      Useful for restore command
// action.saveState[].key       string  ""      Key of value to save, used as {{ key }} in onRestore command
// action.saveState[].default   string  ""      Value to save, used when "cmd" is not set or command execution fails or output is empty
// action.saveState[].command   string  ""      Command that outputs(to stdout) the value to save when executed
// action.saveState[].timeout   number  3000    Timeout for executing "cmd" in milliseconds. 0 or negative means no timeout
// action.onRestore             string  ""      Command to execute on restore ({{ key }} -> value defined or obtained in saveState)
// action.quitOnSelected        boolean false   Whether to quit the application after confirming a wallpaper
// action.restoreOnClose        boolean true    Whether to run the restore command after closing the application without confirming a wallpaper
//
// style.image_width            number  320     Width of each image
// style.image_height           number  180     Height of each image
// style.image_focus_scale      number  1.5     Scale of the focused image (relative to unfocused image)
// style.window_width           number  750     Initial window width
// style.window_height          number  500     Initial window height
//
// sort.type                    string  "date"  Initial sorting type: "name", "date", "size"
// sort.descending              boolean true    Initial sorting order
//                                              Ascending: name: lexicographical, e.g. "a.jpg" before "b.jpg"
//                                                         date: older before newer
//                                                         size: smaller before larger

namespace WallReel::Core::Config {

inline const QString s_DefaultConfigFileName = "config.json";

enum class SortType : int {
    Name,  // "name"
    Date,  // "date"
    Size,  // "size"
};

inline const QStringList s_availableSortTypes = {"Name", "Date", "Size"};

inline QString sortTypeToString(SortType type) {
    switch (type) {
        case SortType::Name:
            return "Name";
        case SortType::Date:
            return "Date";
        case SortType::Size:
            return "Size";
        default:
            return "Date";
    }
}

inline SortType stringToSortType(const QString& str) {
    if (str.compare("name", Qt::CaseInsensitive) == 0) {
        return SortType::Name;
    } else if (str.compare("date", Qt::CaseInsensitive) == 0) {
        return SortType::Date;
    } else if (str.compare("size", Qt::CaseInsensitive) == 0) {
        return SortType::Size;
    } else {
        return SortType::Date;  // default
    }
}

struct WallpaperConfigItems {
    struct WallpaperDirConfigItem {
        QString path;
        bool recursive;
    };

    QStringList paths;
    QList<WallpaperDirConfigItem> dirs;
    QList<QRegularExpression> excludes;
};

struct ThemeConfigItems {
    struct PaletteColorConfigItem {
        QString name;
        QColor value;
    };

    struct PaletteConfigItem {
        QString name;
        QList<PaletteColorConfigItem> colors;
    };

    QList<PaletteConfigItem> palettes;
    QString defaultPalette;
};

struct ActionConfigItems {
    struct SaveStateItem {
        QString key;
        QString defaultVal;
        QString command;
        int timeout = 3000;  // milliseconds, 0 or negative means no timeout
    };

    QList<SaveStateItem> saveStateConfig;
    QHash<QString, QString> savedState;
    QString onSelected;
    QString onPreview;
    QString onRestore;
    int previewDebounceTime = 300;  // milliseconds
    bool printSelected      = true;
    bool printPreview       = false;
    bool quitOnSelected     = false;
    bool restoreOnClose     = true;
};

struct StyleConfigItems {
    double imageFocusScale = 1.5;
    int imageWidth         = 320;
    int imageHeight        = 180;
    int windowWidth        = 750;
    int windowHeight       = 500;
};

struct SortConfigItems {
    SortType type   = SortType::Date;
    bool descending = true;
};

}  // namespace WallReel::Core::Config

#endif  // WALLREEL_CONFIG_DATA_HPP
