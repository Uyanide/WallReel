#ifndef WALLREEL_WALLPAPERSERVICE_HPP
#define WALLREEL_WALLPAPERSERVICE_HPP

#include <QProcess>
#include <QTimer>

#include "configmgr.hpp"
#include "imagedata.hpp"

class WallpaperService : public QObject {
    Q_OBJECT

  public:
    WallpaperService(
        const Config::ActionConfigItems& actionConfig,
        QObject* parent = nullptr);

  public slots:
    void preview(const ImageData& imageData);  // execute after 500ms of inactivity
    void select(const ImageData& imageData);   // execute immediately, ignore if already running
    void restore();                            // execute immediately, ignore if already running

  signals:
    void previewCompleted();
    void selectCompleted();
    void restoreCompleted();

  private:
    void _doPreview(const ImageData& imageData);
    void _doSelect(const ImageData& imageData);
    void _doRestore();

    const Config::ActionConfigItems& m_actionConfig;
    QTimer* m_previewDebounceTimer;
    const ImageData* m_pendingImageData;
    QProcess* m_previewProcess;
    QProcess* m_selectProcess;
    QProcess* m_restoreProcess;
};

#endif  // WALLREEL_WALLPAPERSERVICE_HPP
