#ifndef WALLREEL_WALLPAPERSERVICE_HPP
#define WALLREEL_WALLPAPERSERVICE_HPP

#include <QProcess>
#include <QTimer>

#include "Config/manager.hpp"
#include "Image/data.hpp"

namespace WallReel::Core {

class WallpaperService : public QObject {
    Q_OBJECT

  public:
    WallpaperService(
        const Config::ActionConfigItems& actionConfig,
        QObject* parent = nullptr);

  public slots:
    void preview(const Image::Data& imageData);  // execute after 500ms of inactivity
    void select(const Image::Data& imageData);   // execute immediately, ignore if already running
    void restore();                              // execute immediately, ignore if already running

  signals:
    void previewCompleted();
    void selectCompleted();
    void restoreCompleted();

  private:
    void _doPreview(const Image::Data& imageData);
    void _doSelect(const Image::Data& imageData);
    void _doRestore();

    const Config::ActionConfigItems& m_actionConfig;
    QTimer* m_previewDebounceTimer;
    const Image::Data* m_pendingImageData;
    QProcess* m_previewProcess;
    QProcess* m_selectProcess;
    QProcess* m_restoreProcess;
};

}  // namespace WallReel::Core

#endif  // WALLREEL_WALLPAPERSERVICE_HPP
