#include "Service/wallpaper.hpp"

#include <QColor>

#include "logger.hpp"

WALLREEL_DECLARE_SENDER("WallpaperService")

namespace WallReel::Core::Service {

WallpaperService::WallpaperService(int previewDebounceTime, QObject* parent)
    : QObject(parent) {
    m_previewDebounceTimer = new QTimer(this);
    m_previewDebounceTimer->setSingleShot(true);
    m_previewDebounceTimer->setInterval(previewDebounceTime);
    connect(m_previewDebounceTimer, &QTimer::timeout, this, [this]() {
        _doPreview(m_pendingPreviewCommand);
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

void WallpaperService::stopAll() {
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

void WallpaperService::preview(const QString& command) {
    m_pendingPreviewCommand = command;
    m_previewDebounceTimer->start();
}

void WallpaperService::select(const QString& command) {
    if (m_selectProcess->state() != QProcess::NotRunning) {
        WR_WARN("Previous select command is still running. Ignoring new command.");
        return;
    }
    _doSelect(command);
}

void WallpaperService::restore(const QString& command) {
    if (m_restoreProcess->state() != QProcess::NotRunning) {
        WR_WARN("Previous restore command is still running. Ignoring new command.");
        return;
    }
    WR_DEBUG("Restore state");
    _doRestore(command);
}

void WallpaperService::_doPreview(const QString& command) {
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

void WallpaperService::_doSelect(const QString& command) {
    if (command.isEmpty()) {
        WR_DEBUG("No select command configured. Skipping select action.");
        emit selectCompleted();
        return;
    }
    WR_DEBUG(QString("Executing select command: %1").arg(command));
    m_selectProcess->start("sh", QStringList() << "-c" << command);
}

void WallpaperService::_doRestore(const QString& command) {
    if (command.isEmpty()) {
        WR_DEBUG("Restore command is empty. Skipping restore action.");
        emit restoreCompleted();
        return;
    }
    WR_DEBUG(QString("Executing restore command: %1").arg(command));
    m_restoreProcess->start("sh", QStringList() << "-c" << command);
}

}  // namespace WallReel::Core::Service
