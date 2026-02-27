#ifndef WALLREEL_SERVICE_MANAGER_HPP
#define WALLREEL_SERVICE_MANAGER_HPP

#include <QProcess>
#include <QTimer>

#include "Config/data.hpp"
#include "Image/model.hpp"
#include "Palette/manager.hpp"
#include "Service/wallpaper.hpp"
#include "logger.hpp"

namespace WallReel::Core::Service {

class Manager : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY isProcessingChanged)

  public:
    Manager(
        const Config::ActionConfigItems& actionConfig,
        Image::Model& imageModel,
        Palette::Manager& paletteManager,
        QObject* parent = nullptr) : m_actionConfig(actionConfig), m_imageModel(imageModel), m_paletteManager(paletteManager) {
        m_wallpaperService = new WallpaperService(m_actionConfig, m_paletteManager, this);

        // Listen on image change
        connect(&m_imageModel, &Image::Model::focusedImageChanged, this, &Manager::previewWallpaper);
        // Listen on palette change
        connect(&m_paletteManager, &Palette::Manager::colorChanged, this, &Manager::previewWallpaper);
        connect(&m_paletteManager, &Palette::Manager::colorNameChanged, this, &Manager::previewWallpaper);
        // Forward signals
        // Direct signal 2 signal connection
        connect(m_wallpaperService, &WallpaperService::previewCompleted, this, &Manager::previewCompleted);
        // Signal 2 slot connection to handle processing state
        connect(m_wallpaperService, &WallpaperService::selectCompleted, this, &Manager::_onSelectCompleted);
        connect(m_wallpaperService, &WallpaperService::restoreCompleted, this, &Manager::_onRestoreCompleted);
    }

    Q_INVOKABLE void selectWallpaper(int index) {
        Logger::debug("ServiceManager", QString("Select wallpaper at index %1").arg(index));
        if (m_isProcessing) {
            Logger::debug("ServiceManager", "Already processing an select action, ignoring new request");
            return;
        }
        m_isProcessing = true;
        emit isProcessingChanged();
        const auto* data = m_imageModel.imageAt(index);
        if (data) {
            m_wallpaperService->select(*data);
        } else {
            Logger::warn("ServiceManager", QString("No image data at index %1. Skipping select action.").arg(index));
            m_isProcessing = false;
            emit isProcessingChanged();
            emit selectCompleted();
        }
    }

    Q_INVOKABLE void restore() {
        Logger::debug("ServiceManager", "Restore states");
        if (m_isProcessing) {
            Logger::debug("ServiceManager", "Already processing an restore action, ignoring new request");
            return;
        }
        m_isProcessing = true;
        emit isProcessingChanged();
        m_wallpaperService->restore();
    }

    Q_INVOKABLE void cancel() {
        Logger::debug("ServiceManager", "Cancel action");
        m_wallpaperService->stopAll();
        emit cancelCompleted();
    }

    bool isProcessing() const { return m_isProcessing; }

    bool hasSelected() const { return m_hasSelected; }

  public slots:

    void previewWallpaper() {
        Logger::debug("ServiceManager", "Preview wallpaper");
        const auto* data = m_imageModel.focusedImage();
        if (data) {
            m_wallpaperService->preview(*data);
        } else {
            Logger::warn("ServiceManager", "No focused image data. Skipping preview action.");
            emit previewCompleted();
        }
    }

  private slots:

    void _onSelectCompleted() {
        Logger::debug("ServiceManager", "Select completed");
        _onProcessCompleted();
        m_hasSelected = true;
        emit selectCompleted();
    }

    void _onRestoreCompleted() {
        Logger::debug("ServiceManager", "Restore completed");
        _onProcessCompleted();
        emit restoreCompleted();
    }

    void _onProcessCompleted() {
        m_isProcessing = false;
        emit isProcessingChanged();
    }

  signals:
    void isProcessingChanged();
    void selectCompleted();
    void previewCompleted();
    void restoreCompleted();
    void cancelCompleted();

  private:
    WallpaperService* m_wallpaperService;
    const Config::ActionConfigItems& m_actionConfig;
    Image::Model& m_imageModel;
    Palette::Manager& m_paletteManager;

    bool m_isProcessing = false;
    bool m_hasSelected  = false;
};

}  // namespace WallReel::Core::Service

#endif  // WALLREEL_SERVICE_MANAGER_HPP
