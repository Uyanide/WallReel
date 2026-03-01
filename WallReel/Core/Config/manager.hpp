#ifndef WALLREEL_CONFIGMGR_HPP
#define WALLREEL_CONFIGMGR_HPP

#include <QDir>

#include "data.hpp"

namespace WallReel::Core::Config {

/**
 * @brief Config Manager (QML Singleton)
 *
 * @details Config Manager, which:
 * - Loads and parses the configuration file
 * - Provides access to configuration values via getters and Q_PROPERTY
 * - Scans for wallpapers based on the configuration
 * - Captures state when requested and emits a signal when done
 */
class Manager : public QObject {
    Q_OBJECT

  public:
    /**
     * @brief Construct a new Manager object
     *
     * @param configDir The directory where the configuration file is located (should be the default location, i.e. $XDG_CONFIG_HOME/$appName)
     * @param searchDirs Additional directories to search for wallpapers (not recursive)
     * @param configPath Optional path to a specific configuration file (overrides the default config path)
     * @param picturesDir The pictures directory (default location for user wallpapers)
     * @param parent QObject parent
     *
     * @note The constructor will load the configuration and scan for wallpapers immediately.
     */
    Manager(
        const QDir& configDir,
        const QDir& picturesDir,
        const QStringList& searchDirs = {},
        const QString& configPath     = "",
        QObject* parent               = nullptr);

    ~Manager();

    /**
     * @brief Getter, Get the list of wallpapers found
     *
     * @return const QStringList&
     */
    const QStringList& getWallpapers() const { return m_wallpapers; }

    // Separate getters for each field in the configuration

    const WallpaperConfigItems& getWallpaperConfig() const { return m_wallpaperConfig; }

    const ThemeConfigItems& getThemeConfig() const { return m_themeConfig; }

    const ActionConfigItems& getActionConfig() const { return m_actionConfig; }

    const StyleConfigItems& getStyleConfig() const { return m_styleConfig; }

    const CacheConfigItems& getCacheConfig() const { return m_cacheConfig; }

    QSize getFocusImageSize() const {
        return QSize{m_styleConfig.imageWidth, m_styleConfig.imageHeight} * m_styleConfig.imageFocusScale;
    }

    /**
     * @brief Capture the current state of the configuration and emit stateCaptured() when done
     */
    Q_INVOKABLE void captureState();

  signals:
    void stateCaptured();

  private:
    // Parse config
    void _loadConfig(const QString& configPath);
    void _loadWallpaperConfig(const QJsonObject& config);
    void _loadThemeConfig(const QJsonObject& config);
    void _loadActionConfig(const QJsonObject& config);
    void _loadStyleConfig(const QJsonObject& config);
    void _loadCacheConfig(const QJsonObject& config);
    // Load wallpapers
    void _loadWallpapers();
    // Callback for state capture results
    void _onCaptureResult(const QString& key, const QString& value);

  private:
    const QDir m_configDir;
    WallpaperConfigItems m_wallpaperConfig;
    ThemeConfigItems m_themeConfig;
    ActionConfigItems m_actionConfig;
    StyleConfigItems m_styleConfig;
    CacheConfigItems m_cacheConfig;

    QStringList m_wallpapers;

    int m_pendingCaptures = 0;
};

}  // namespace WallReel::Core::Config

#endif  // WALLREEL_CONFIGMGR_HPP
