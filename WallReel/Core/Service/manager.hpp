#ifndef WALLREEL_SERVICE_MANAGER_HPP
#define WALLREEL_SERVICE_MANAGER_HPP

#include <QProcess>
#include <QTimer>

#include "Config/data.hpp"
#include "Image/model.hpp"
#include "Service/wallpaper.hpp"
#include "logger.hpp"

namespace WallReel::Core::Service {

class Manager : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY isProcessingChanged)

  public:
    Manager(
        const Config::ActionConfigItems& actionConfig,
        const Image::Model& imageModel,
        QObject* parent = nullptr) : m_imageModel(imageModel) {
        m_wallpaperService = new WallpaperService(actionConfig, this);

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
        const auto* data = m_imageModel.getDataPtrAt(index);
        if (data) {
            m_wallpaperService->select(*data);
        } else {
            m_isProcessing = false;
            emit isProcessingChanged();
            emit selectCompleted();
        }
    }

    Q_INVOKABLE void previewWallpaper(int index) {
        const auto* data = m_imageModel.getDataPtrAt(index);
        if (data) {
            m_wallpaperService->preview(*data);
        } else {
            emit previewCompleted();
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

    bool isProcessing() const { return m_isProcessing; }

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

  private:
    WallpaperService* m_wallpaperService;
    const Image::Model& m_imageModel;

    bool m_isProcessing = false;
};

}  // namespace WallReel::Core::Service

#endif  // WALLREEL_SERVICE_MANAGER_HPP
