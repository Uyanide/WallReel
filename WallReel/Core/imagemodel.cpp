
#include "imagemodel.hpp"

#include <QFuture>
#include <QtConcurrent>

#include "imagedata.hpp"

ImageModel::ImageModel(
    ImageProvider& provider,
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
        &ImageModel::_onProcessingFinished);
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
    m_data.clear();
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

    _clearData();

    m_processedCount = 0;
    m_progressUpdateTimer.start(s_ProgressUpdateIntervalMs);
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

void ImageModel::_onProgressValueChanged(int value) {
    Q_UNUSED(value);
    emit progressChanged();
}

void ImageModel::_onProcessingFinished() {
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
    QTimer::singleShot(s_IsLoadingUpdateIntervalMs, this, [this]() {
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
        if (a == b) {
            return false;
        }

        ImageData* first  = reverse ? b : a;
        ImageData* second = reverse ? a : b;

        switch (type) {
            case Config::SortType::Name:
                return QString::compare(first->getFileName(), second->getFileName(), Qt::CaseInsensitive) < 0;
            case Config::SortType::Date:
                return first->getLastModified() < second->getLastModified();
            case Config::SortType::Size:
                return first->getSize() < second->getSize();
            default:
                return false;
        }
    });

    beginResetModel();
    m_provider.clear();
    for (const auto& item : m_data) {
        m_provider.insert(item);
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

void ImageModel::_clearData() {
    beginResetModel();
    m_provider.clear();
    qDeleteAll(m_data);
    m_data.clear();
    endResetModel();
}

void ImageModel::selectImage(int index) {
    if (index < 0 || index >= m_data.count()) {
        return;
    }
    const auto& item = m_data[index];
    if (item) {
        emit imageSelected(*item);
    }
}

void ImageModel::previewImage(int index) {
    if (index < 0 || index >= m_data.count()) {
        return;
    }
    const auto& item = m_data[index];
    if (item) {
        emit imagePreviewed(*item);
    }
}
