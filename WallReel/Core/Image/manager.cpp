#include "manager.hpp"

#include <QFuture>
#include <QtConcurrent>

#include "data.hpp"
#include "logger.hpp"

WALLREEL_DECLARE_SENDER("ImageManager")

WallReel::Core::Image::Manager::Manager(
    Cache::Manager& cacheMgr,
    const QSize& thumbnailSize,
    QObject* parent)
    : QObject(parent),
      m_cacheMgr(cacheMgr),
      m_thumbnailSize(thumbnailSize) {
    m_dataModel  = new Model(this);
    m_proxyModel = new ProxyModel(this);
    m_proxyModel->setSourceModel(m_dataModel);

    connect(
        &m_watcher,
        &QFutureWatcher<Data*>::finished,
        this,
        &Manager::_onProcessingFinished);
    connect(
        &m_progressUpdateTimer,
        &QTimer::timeout,
        this,
        [this]() {
            emit processedCountChanged();
        });
}

WallReel::Core::Image::Manager::~Manager() {
    m_watcher.cancel();
    m_watcher.waitForFinished();
}

void WallReel::Core::Image::Manager::loadAndProcess(const QStringList& paths) {
    if (m_isLoading) {
        WR_WARN("Already loading images. Ignoring new load request.");
        return;
    }
    m_isLoading = true;
    emit isLoadingChanged();

    _clearData();

    m_processedCount = 0;
    m_progressUpdateTimer.start(s_ProgressUpdateIntervalMs);
    // These are all small objects so capturing by value should be fine
    const auto thumbnailSize = m_thumbnailSize;
    const auto counterPtr    = &m_processedCount;
    const auto cacheMgr      = &m_cacheMgr;
    QFuture<Data*> future =
        QtConcurrent::mapped(paths, [thumbnailSize, counterPtr, cacheMgr](const QString& path) {
            auto data = Data::create(path, thumbnailSize, *cacheMgr);
            counterPtr->fetch_add(1, std::memory_order_relaxed);
            return data;
        });
    m_watcher.setFuture(future);
    emit totalCountChanged();
}

void WallReel::Core::Image::Manager::stop() {
    if (m_isLoading) {
        WR_INFO("Stopping image loading...");
        m_watcher.cancel();
    } else {
        WR_WARN("No loading operation to stop.");
    }
}

void WallReel::Core::Image::Manager::_clearData() {
    m_dataModel->clearData();
}

void WallReel::Core::Image::Manager::_onProgressValueChanged(int value) {
    Q_UNUSED(value);
    emit processedCountChanged();
}

void WallReel::Core::Image::Manager::_onProcessingFinished() {
    auto results = m_watcher.future().results();

    QList<Data*> filteredResults;
    filteredResults.reserve(results.size());
    for (Data* data : results) {
        if (data && data->isValid()) {
            filteredResults.append(data);
            m_dataMap.insert(data->getId(), data);
        } else {
            if (data) {
                WR_WARN(QString("Failed to load image data for path '%1'").arg(data->getFullPath()));
                delete data;
            }
        }
    }

    m_dataModel->insertData(filteredResults);

    WR_INFO("Finished loading images. Total valid images: " + QString::number(filteredResults.size()));

    m_isLoading = false;
    m_progressUpdateTimer.stop();
    emit processedCountChanged();
    // QTimer::singleShot(s_IsLoadingUpdateIntervalMs, this, [this]() {
    //     emit isLoadingChanged();
    // });
    emit isLoadingChanged();
}
