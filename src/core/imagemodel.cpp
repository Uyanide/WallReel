
#include "imagemodel.hpp"

#include <QFuture>
#include <QtConcurrent>

#include "imagedata.hpp"

ImageModel::ImageModel(
    ImageProvider* provider,
    const Config::SortConfigItems& sortConfig,
    QSize thumbnailSize,
    QObject* parent)
    : QAbstractListModel(parent),
      m_provider(provider),
      m_sortConfig(sortConfig),
      m_thumbnailSize(thumbnailSize) {
    connect(
        &m_watcher,
        &QFutureWatcher<ImageData*>::finished,
        this,
        &ImageModel::onProcessingFinished);
    connect(
        &m_progressUpdateTimer,
        &QTimer::timeout,
        this,
        [this]() {
            emit progressChanged();
        });
}

ImageModel::~ImageModel() {
    m_watcher.cancel();
    m_watcher.waitForFinished();
    qDeleteAll(m_data);
}

int ImageModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_data.count();
}

QVariant ImageModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_data.count()) {
        return QVariant();
    }

    const auto& item = m_data[index.row()];
    switch (role) {
        case IdRole:
            return item->getId();
        case PathRole:
            return item->getFullPath();
        case NameRole:
            return item->getFileName();
        default:
            return QVariant();
    }
}

void ImageModel::loadAndProcess(const QStringList& paths) {
    if (m_isLoading) {
        return;
    }
    m_isLoading = true;
    emit isLoadingChanged();

    beginResetModel();
    if (!m_data.isEmpty()) {
        qDeleteAll(m_data);
    }
    m_data.clear();
    m_provider->clear();
    endResetModel();

    m_processedCount = 0;
    m_progressUpdateTimer.start(s_progressUpdateInterval);
    const auto thumbnailSize   = m_thumbnailSize;
    const auto counterPtr      = &m_processedCount;
    QFuture<ImageData*> future = QtConcurrent::mapped(paths, [thumbnailSize, counterPtr](const QString& path) {
        auto data = ImageData::create(path, thumbnailSize);
        counterPtr->fetch_add(1, std::memory_order_relaxed);
        return data;
    });
    m_watcher.setFuture(future);
    emit totalCountChanged();
}

void ImageModel::stop() {
    if (m_isLoading) {
        m_watcher.cancel();
    }
}

void ImageModel::onProgressValueChanged(int value) {
    Q_UNUSED(value);
    emit progressChanged();
}

void ImageModel::onProcessingFinished() {
    auto results = m_watcher.future().results();
    for (auto& data : results) {
        if (data && data->isValid()) {
            m_data.append(data);
        } else {
            delete data;
            data = nullptr;
        }
    }

    sortUpdate();

    m_isLoading = false;
    m_progressUpdateTimer.stop();
    emit progressChanged();
    // emit isLoadingChanged();
    QTimer::singleShot(s_isLoadingUpdateInterval, this, [this]() {
        emit isLoadingChanged();
    });
}

void ImageModel::sortUpdate() {
    const auto type    = m_sortConfig.type;
    const auto reverse = m_sortConfig.reverse;
    std::sort(m_data.begin(), m_data.end(), [type, reverse](ImageData* a, ImageData* b) {
        if (!a || !b) {
            return false;
        }
        bool result = false;
        switch (type) {
            case Config::SortType::Name:
                result = QString::compare(a->getFileName(), b->getFileName(), Qt::CaseInsensitive) < 0;
                break;
            case Config::SortType::Date:
                result = a->getLastModified() < b->getLastModified();
                break;
            case Config::SortType::Size:
                result = a->getSize() < b->getSize();
                break;
            default:
                break;
        }
        return reverse ? !result : result;
    });

    beginResetModel();
    m_provider->clear();
    for (const auto& item : m_data) {
        m_provider->insert(item);
    }
    endResetModel();
}

QVariant ImageModel::dataAt(int index, const QString& roleName) const {
    if (index < 0 || index >= m_data.count()) {
        return QVariant();
    }

    const auto& item = m_data[index];
    if (roleName == "imgId") {
        return item->getId();
    } else if (roleName == "imgPath") {
        return item->getFullPath();
    } else if (roleName == "imgName") {
        return item->getFileName();
    } else {
        return QVariant();
    }
}
