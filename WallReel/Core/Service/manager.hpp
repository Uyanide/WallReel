#ifndef WALLREEL_SERVICE_MANAGER_HPP
#define WALLREEL_SERVICE_MANAGER_HPP

#include <QProcess>
#include <QTimer>

#include "Config/data.hpp"
#include "Image/manager.hpp"
#include "Palette/manager.hpp"
#include "Service/wallpaper.hpp"

namespace WallReel::Core::Service {

class Manager : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY isProcessingChanged)

  public:
    Manager(
        const Config::ActionConfigItems& actionConfig,
        Image::Manager& imageManager,
        Palette::Manager& paletteManager,
        QObject* parent = nullptr);

    bool isProcessing() const { return m_isProcessing; }

    bool hasSelected() const { return m_hasSelected; }

  public slots:

    void onStateCaptured();

    void selectWallpaper(const QString& id);

    void restore();

    void cancel();

    void previewWallpaper(const QString& id);

    void restoreOnQuit();

  private slots:

    void _onSelectCompleted();

    void _onRestoreCompleted();

    void _onProcessCompleted();

  signals:
    void isProcessingChanged();
    void selectCompleted();
    void previewCompleted();
    void restoreCompleted();
    void cancelCompleted();

  private:
    QString _renderCommand(const QString& templateStr, const QHash<QString, QString>& variables) const;
    QHash<QString, QString> _generateVariables(const Image::Data& imageData) const;

  private:
    WallpaperService* m_wallpaperService;
    const Config::ActionConfigItems& m_actionConfig;
    Image::Manager& m_imageManager;
    Palette::Manager& m_paletteManager;

    bool m_isProcessing = false;
    bool m_hasSelected  = false;

    bool m_stateCaptured = false;
    QString m_pendingPreviewId;
};

}  // namespace WallReel::Core::Service

#endif  // WALLREEL_SERVICE_MANAGER_HPP
