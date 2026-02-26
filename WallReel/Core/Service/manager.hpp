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
        if (m_isProcessing) {
            Logger::debug("Already processing an action, ignoring select request");
            return;
        }
        m_isProcessing = true;
        emit isProcessingChanged();
        const auto* data = m_imageModel.imageAt(index);
        if (data) {
            m_wallpaperService->select(*data);
        } else {
            m_isProcessing = false;
            emit isProcessingChanged();
            emit selectCompleted();
        }
    }

    Q_INVOKABLE void restore() {
        if (m_isProcessing) {
            Logger::debug("Already processing an action, ignoring restore request");
            return;
        }
        m_isProcessing = true;
        emit isProcessingChanged();
        m_wallpaperService->restore();
    }

    Q_INVOKABLE void cancel() {
        m_wallpaperService->stopAll();
        if (m_actionConfig.restoreOnCancel) {
            connect(m_wallpaperService, &WallpaperService::restoreCompleted, this, [this]() {
                emit cancelCompleted();
            });
            restore();
        } else {
            emit cancelCompleted();
        }
    }

    bool isProcessing() const { return m_isProcessing; }

  public slots:

    void previewWallpaper() {
        const auto* data = m_imageModel.focusedImage();
        if (data) {
            m_wallpaperService->preview(*data);
        } else {
            emit previewCompleted();
        }
    }

  private slots:

    void _onSelectCompleted() {
        _onProcessCompleted();
        emit selectCompleted();
    }

    void _onRestoreCompleted() {
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
};

}  // namespace WallReel::Core::Service

#endif  // WALLREEL_SERVICE_MANAGER_HPP
