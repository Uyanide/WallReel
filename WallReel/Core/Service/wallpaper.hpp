#ifndef WALLREEL_WALLPAPERSERVICE_HPP
#define WALLREEL_WALLPAPERSERVICE_HPP

#include <QProcess>
#include <QTimer>

#include "Config/data.hpp"
#include "Image/data.hpp"
#include "Palette/manager.hpp"

namespace WallReel::Core::Service {

class WallpaperService : public QObject {
    Q_OBJECT

  public:
    WallpaperService(
        const Config::ActionConfigItems& actionConfig,
        const Palette::Manager& paletteManager,
        QObject* parent = nullptr);

    void stopAll();

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
    QHash<QString, QString> _generateVariables(const Image::Data& imageData);

    const Config::ActionConfigItems& m_actionConfig;
    const Palette::Manager& m_paletteManager;
    QTimer* m_previewDebounceTimer;
    const Image::Data* m_pendingImageData;
    QProcess* m_previewProcess;
    QProcess* m_selectProcess;
    QProcess* m_restoreProcess;
};

}  // namespace WallReel::Core::Service

#endif  // WALLREEL_WALLPAPERSERVICE_HPP
