#include "manager.hpp"

#include "Utils/texttemplate.hpp"
#include "logger.hpp"

WALLREEL_DECLARE_SENDER("ServiceManager")

namespace WallReel::Core::Service {

Manager::Manager(
    const Config::ActionConfigItems& actionConfig,
    Image::Manager& imageManager,
    Palette::Manager& paletteManager,
    QObject* parent) : m_actionConfig(actionConfig), m_imageManager(imageManager), m_paletteManager(paletteManager) {
    m_wallpaperService = new WallpaperService(m_actionConfig.previewDebounceTime, this);

    // Forward signals
    // Direct signal 2 signal connection
    connect(m_wallpaperService, &WallpaperService::previewCompleted, this, &Manager::previewCompleted);
    // Signal 2 slot connection to handle processing state
    connect(m_wallpaperService, &WallpaperService::selectCompleted, this, &Manager::_onSelectCompleted);
    connect(m_wallpaperService, &WallpaperService::restoreCompleted, this, &Manager::_onRestoreCompleted);
}

void Manager::onStateCaptured() {
    m_stateCaptured = true;

    if (!m_pendingPreviewId.isEmpty()) {
        WR_DEBUG("State captured, executing pending preview for id " + m_pendingPreviewId);
        const QString pending = m_pendingPreviewId;
        m_pendingPreviewId.clear();
        previewWallpaper(pending);
    }
}

void Manager::selectWallpaper(const QString& id) {
    WR_DEBUG("Select action triggered for id " + id);
    if (m_isProcessing) {
        WR_DEBUG("Already processing an select action, ignoring new request");
        return;
    }
    m_isProcessing = true;
    emit isProcessingChanged();
    const auto* data = m_imageManager.imageAt(id);

    if (!data || !data->isValid()) {
        WR_WARN(QString("No valid image data at id %1. Skipping select action.").arg(id));
        m_isProcessing = false;
        emit isProcessingChanged();
        emit selectCompleted();
    }

    const auto command = _renderCommand(m_actionConfig.onSelected, _generateVariables(*data));
    m_wallpaperService->select(command);
}

void Manager::restore() {
    WR_DEBUG("Restore action triggered");
    if (m_isProcessing) {
        WR_DEBUG("Already processing an restore action, ignoring new request");
        return;
    }
    if (!m_stateCaptured) {
        WR_DEBUG("State not captured yet, skipping restore action");
        emit restoreCompleted();
        return;
    }
    m_isProcessing = true;
    emit isProcessingChanged();

    m_wallpaperService->restore(_renderCommand(m_actionConfig.onRestore, m_actionConfig.savedState));
}

void Manager::cancel() {
    WR_DEBUG("Cancel action triggered");
    m_wallpaperService->stopAll();
    emit cancelCompleted();
}

void Manager::previewWallpaper(const QString& id) {
    if (!m_stateCaptured) {
        WR_DEBUG("State not captured yet, deferring preview for id " + id);
        m_pendingPreviewId = id;
        emit previewCompleted();
        return;
    }

    WR_DEBUG("Preview action triggered for id " + id);

    const auto* data = m_imageManager.imageAt(id);

    if (!data || !data->isValid()) {
        WR_WARN(QString("No valid image data at id %1. Skipping preview action.").arg(id));
        emit previewCompleted();
        return;
    }

    m_wallpaperService->preview(_renderCommand(m_actionConfig.onPreview, _generateVariables(*data)));
}

void Manager::restoreOnQuit() {
    if (m_hasSelected) {
        Logger::debug("ServiceManager", "Quit with selected wallpaper, no need to restore");
        return;
    }
    Logger::debug("ServiceManager", "Restore on quit");
    m_wallpaperService->stopAll();
    QEventLoop loop;
    connect(m_wallpaperService, &WallpaperService::restoreCompleted, &loop, &QEventLoop::quit);
    // Call restore after the event loop starts
    QTimer::singleShot(0, this, &Manager::restore);
    loop.exec();
}

void Manager::_onSelectCompleted() {
    Logger::debug("ServiceManager", "Select completed");
    _onProcessCompleted();
    m_hasSelected = true;
    emit selectCompleted();
}

void Manager::_onRestoreCompleted() {
    Logger::debug("ServiceManager", "Restore completed");
    _onProcessCompleted();
    emit restoreCompleted();
}

void Manager::_onProcessCompleted() {
    m_isProcessing = false;
    emit isProcessingChanged();
}

QString Manager::_renderCommand(const QString& templateStr, const QHash<QString, QString>& variables) const {
    return Utils::renderTemplate(templateStr, variables);
}

QHash<QString, QString> Manager::_generateVariables(const Image::Data& imageData) const {
    auto palette = m_paletteManager.getSelectedPaletteName();
    if (palette.isEmpty()) {
        palette = "null";
    }
    auto color = m_paletteManager.getCurrentColorName();
    if (color.isEmpty()) {
        color = "null";
    }
    auto hex = m_paletteManager.getCurrentColorHex();
    if (hex.isEmpty()) {
        hex = "null";
    }
    QHash<QString, QString> ret{
        {"path", imageData.getFullPath()},
        {"name", imageData.getFileName()},
        {"size", QString::number(imageData.getSize())},
        {"palette", palette},
        {"colorName", color},
        {"colorHex", hex},
        {"domColorHex", imageData.getDominantColor().name()},
    };

    ret.insert(m_actionConfig.savedState);
    return ret;
}

}  // namespace WallReel::Core::Service
