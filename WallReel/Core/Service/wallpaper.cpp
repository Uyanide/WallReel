#include "Service/wallpaper.hpp"

#include <QColor>
#include <iostream>

#include "Utils/texttemplate.hpp"
#include "logger.hpp"

WALLREEL_DECLARE_SENDER("WallpaperService")

WallReel::Core::Service::WallpaperService::WallpaperService(
    const Config::ActionConfigItems& actionConfig,
    const Palette::Manager& paletteManager,
    QObject* parent)
    : QObject(parent), m_actionConfig(actionConfig), m_paletteManager(paletteManager) {
    m_previewDebounceTimer = new QTimer(this);
    m_previewDebounceTimer->setSingleShot(true);
    m_previewDebounceTimer->setInterval(m_actionConfig.previewDebounceTime);
    connect(m_previewDebounceTimer, &QTimer::timeout, this, [this]() {
        _doPreview(*m_pendingImageData);
    });

    m_previewProcess = new QProcess(this);
    connect(m_previewProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                WR_DEBUG(QString("Preview process finished with exit code %1 and exit status %2").arg(exitCode).arg(exitStatus));
                emit previewCompleted();
            });

    m_selectProcess = new QProcess(this);
    connect(m_selectProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                WR_DEBUG(QString("Select process finished with exit code %1 and exit status %2").arg(exitCode).arg(exitStatus));
                emit selectCompleted();
            });

    m_restoreProcess = new QProcess(this);
    connect(m_restoreProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                WR_DEBUG(QString("Restore process finished with exit code %1 and exit status %2").arg(exitCode).arg(exitStatus));
                emit restoreCompleted();
            });
}

void WallReel::Core::Service::WallpaperService::stopAll() {
    WR_DEBUG("Stopping all wallpaper service processes");
    if (m_previewProcess->state() != QProcess::NotRunning) {
        m_previewProcess->kill();
        m_previewProcess->waitForFinished();
    }
    if (m_selectProcess->state() != QProcess::NotRunning) {
        m_selectProcess->kill();
        m_selectProcess->waitForFinished();
    }
    if (m_restoreProcess->state() != QProcess::NotRunning) {
        m_restoreProcess->kill();
        m_restoreProcess->waitForFinished();
    }
    m_previewDebounceTimer->stop();
}

void WallReel::Core::Service::WallpaperService::preview(const Image::Data& imageData) {
    m_pendingImageData = &imageData;
    m_previewDebounceTimer->start();
}

void WallReel::Core::Service::WallpaperService::select(const Image::Data& imageData) {
    if (m_selectProcess->state() != QProcess::NotRunning) {
        WR_WARN("Previous select command is still running. Ignoring new command.");
        return;
    }
    WR_DEBUG(QString("Select wallpaper: %1").arg(imageData.getFullPath()));
    _doSelect(imageData);
}

void WallReel::Core::Service::WallpaperService::restore() {
    if (m_restoreProcess->state() != QProcess::NotRunning) {
        WR_WARN("Previous restore command is still running. Ignoring new command.");
        return;
    }
    WR_DEBUG("Restore state");
    _doRestore();
}

QHash<QString, QString> WallReel::Core::Service::WallpaperService::_generateVariables(const Image::Data& imageData) {
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

void WallReel::Core::Service::WallpaperService::_doPreview(const Image::Data& imageData) {
    QString path = imageData.getFullPath();

    if (path.isEmpty()) {
        WR_WARN("No valid image path for preview. Skipping preview action.");
        emit previewCompleted();
        return;
    }

    if (m_actionConfig.printPreview) {
        std::cout << path.toStdString() << std::endl;
    }

    const auto variables = _generateVariables(imageData);
    auto command         = Utils::renderTemplate(m_actionConfig.onPreview, variables);
    if (command.isEmpty()) {
        WR_DEBUG("No preview command configured. Skipping preview action.");
        emit previewCompleted();
        return;
    }
    WR_DEBUG(QString("Executing preview command: %1").arg(command));

    if (m_previewProcess->state() != QProcess::NotRunning) {
        m_previewProcess->kill();
        m_previewProcess->waitForFinished();
    }
    m_previewProcess->start("sh", QStringList() << "-c" << command);
}

void WallReel::Core::Service::WallpaperService::_doSelect(const Image::Data& imageData) {
    QString path = imageData.getFullPath();

    if (path.isEmpty()) {
        WR_WARN("No valid image path for select. Skipping select action.");
        emit selectCompleted();
        return;
    }

    if (m_actionConfig.printSelected) {
        std::cout << path.toStdString() << std::endl;
    }

    const auto variables = _generateVariables(imageData);
    auto command         = Utils::renderTemplate(m_actionConfig.onSelected, variables);
    if (command.isEmpty()) {
        WR_DEBUG("No select command configured. Skipping select action.");
        emit selectCompleted();
        return;
    }
    WR_DEBUG(QString("Executing select command: %1").arg(command));
    m_selectProcess->start("sh", QStringList() << "-c" << command);
}

void WallReel::Core::Service::WallpaperService::_doRestore() {
    if (m_actionConfig.onRestore.isEmpty()) {
        WR_DEBUG("No restore command configured. Skipping restore action.");
        emit restoreCompleted();
        return;
    }

    const QString command = Utils::renderTemplate(m_actionConfig.onRestore, m_actionConfig.savedState);
    if (command.isEmpty()) {
        WR_DEBUG("Restore command is empty after rendering. Skipping restore action.");
        emit restoreCompleted();
        return;
    }
    WR_DEBUG(QString("Executing restore command: %1").arg(command));
    m_restoreProcess->start("sh", QStringList() << "-c" << command);
}
