#include "wallpaperservice.hpp"

#include <QColor>
#include <iostream>

#include "utils/logger.hpp"
#include "utils/texttemplate.hpp"

using namespace GeneralLogger;

WallpaperService::WallpaperService(
    const Config::ActionConfigItems& actionConfig,
    QObject* parent)
    : QObject(parent), m_actionConfig(actionConfig) {
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
                Q_UNUSED(exitCode);
                Q_UNUSED(exitStatus);
                emit previewCompleted();
            });

    m_selectProcess = new QProcess(this);
    connect(m_selectProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                Q_UNUSED(exitCode);
                Q_UNUSED(exitStatus);
                emit selectCompleted();
            });

    m_restoreProcess = new QProcess(this);
    connect(m_restoreProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                Q_UNUSED(exitCode);
                Q_UNUSED(exitStatus);
                emit restoreCompleted();
            });
}

void WallpaperService::preview(const ImageData& imageData) {
    m_pendingImageData = &imageData;
    m_previewDebounceTimer->start();
}

void WallpaperService::select(const ImageData& imageData) {
    if (m_selectProcess->state() != QProcess::NotRunning) {
        warn("Previous select command is still running. Ignoring new command.");
        return;
    }
    _doSelect(imageData);
}

void WallpaperService::restore() {
    if (m_restoreProcess->state() != QProcess::NotRunning) {
        warn("Previous restore command is still running. Ignoring new command.");
        return;
    }
    _doRestore();
}

void WallpaperService::_doPreview(const ImageData& imageData) {
    QString path = imageData.getFullPath();

    if (path.isEmpty()) {
        return;
    }

    if (m_actionConfig.printPreview) {
        std::cout << path.toStdString() << std::endl;
    }

    const QHash<QString, QString> variables{
        {"path", path},
        {"name", imageData.getFileName()},
    };
    auto command = renderTemplate(m_actionConfig.onPreview, variables);
    if (command.isEmpty()) {
        return;
    }

    if (m_previewProcess->state() != QProcess::NotRunning) {
        m_previewProcess->kill();
        m_previewProcess->waitForFinished();
    }
    m_previewProcess->start("sh", QStringList() << "-c" << command);
}

void WallpaperService::_doSelect(const ImageData& imageData) {
    QString path = imageData.getFullPath();

    if (path.isEmpty()) {
        return;
    }

    if (m_actionConfig.printSelected) {
        std::cout << path.toStdString() << std::endl;
    }

    const QHash<QString, QString> variables{
        {"path", path},
        {"name", imageData.getFileName()},
    };
    auto command = renderTemplate(m_actionConfig.onSelected, variables);
    if (command.isEmpty()) {
        return;
    }
    m_selectProcess->start("sh", QStringList() << "-c" << command);
}

void WallpaperService::_doRestore() {
    if (m_actionConfig.onRestore.isEmpty()) {
        return;
    }

    const QString command = renderTemplate(m_actionConfig.onRestore, m_actionConfig.saveState);
    if (command.isEmpty()) {
        return;
    }
    m_restoreProcess->start("sh", QStringList() << "-c" << command);
}
