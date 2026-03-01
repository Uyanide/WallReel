#ifndef WALLREEL_WALLPAPERSERVICE_HPP
#define WALLREEL_WALLPAPERSERVICE_HPP

#include <QProcess>
#include <QTimer>

namespace WallReel::Core::Service {

class WallpaperService : public QObject {
    Q_OBJECT

  public:
    WallpaperService(int previewDebounceTime, QObject* parent = nullptr);

    void stopAll();

  public slots:
    void preview(const QString& command);  // execute after 500ms of inactivity
    void select(const QString& command);   // execute immediately, ignore if already running
    void restore(const QString& command);  // execute immediately, ignore if already running

  signals:
    void previewCompleted();
    void selectCompleted();
    void restoreCompleted();

  private:
    void _doPreview(const QString& command);
    void _doSelect(const QString& command);
    void _doRestore(const QString& command);

    QTimer* m_previewDebounceTimer;
    QString m_pendingPreviewCommand;
    QProcess* m_previewProcess;
    QProcess* m_selectProcess;
    QProcess* m_restoreProcess;
};

}  // namespace WallReel::Core::Service

#endif  // WALLREEL_WALLPAPERSERVICE_HPP
