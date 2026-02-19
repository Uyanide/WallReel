#ifndef WALLREEL_CONFIGMGR_HPP
#define WALLREEL_CONFIGMGR_HPP

#include "data.hpp"

namespace WallReel::Core::Config {

class Manager : public QObject {
    Q_OBJECT

    Q_PROPERTY(int imageWidth READ getImageWidth CONSTANT)
    Q_PROPERTY(int imageHeight READ getImageHeight CONSTANT)
    Q_PROPERTY(double imageFocusScale READ getImageFocusScale CONSTANT)
    Q_PROPERTY(int windowWidth READ getWindowWidth CONSTANT)
    Q_PROPERTY(int windowHeight READ getWindowHeight CONSTANT)

  public:
    Manager(
        const QString& configDir,
        const QStringList& searchDirs = {},
        const QString& configPath     = "",  // Override the default config path
        QObject* parent               = nullptr);

    ~Manager();

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

    Q_INVOKABLE void captureState();

  signals:
    void stateCaptured();

  private:
    void _loadConfig(const QString& configPath);
    void _loadWallpapers();
    void _loadWallpaperConfig(const QJsonObject& config);
    void _loadPaletteConfig(const QJsonObject& config);
    void _loadActionConfig(const QJsonObject& config);
    void _loadStyleConfig(const QJsonObject& config);
    void _loadSortConfig(const QJsonObject& config);
    void _onCaptureResult(const QString& key, const QString& value);

  private:
    const QString m_configDir;
    WallpaperConfigItems m_wallpaperConfig;
    PaletteConfigItems m_paletteConfig;
    ActionConfigItems m_actionConfig;
    StyleConfigItems m_styleConfig;
    SortConfigItems m_sortConfig;

    QStringList m_wallpapers;

    int m_pendingCaptures = 0;
};

}  // namespace WallReel::Core::Config

#endif  // WALLREEL_CONFIGMGR_HPP
