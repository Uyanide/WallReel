#include "model.hpp"

#include <QFuture>
#include <QtConcurrent>

#include "data.hpp"
#include "logger.hpp"

WallReel::Core::Image::Model::Model(
    Provider& provider,
    const Config::SortConfigItems& sortConfig,
    QSize thumbnailSize,
    QObject* parent)
    : QAbstractListModel(parent),
      m_provider(provider),
      m_sortConfig(sortConfig),
      m_thumbnailSize(thumbnailSize),
      m_currentSortType(sortConfig.type) {
    connect(
        &m_watcher,
        &QFutureWatcher<Data*>::finished,
        this,
        &Model::_onProcessingFinished);
    connect(
        &m_progressUpdateTimer,
        &QTimer::timeout,
        this,
        [this]() {
            emit progressChanged();
        });

    // Update state when sort method changes
    connect(this, &Model::currentSortTypeChanged, this, &Model::_onSortMethodChanged);
    connect(this, &Model::currentSortReverseChanged, this, &Model::_onSortMethodChanged);

    m_sortIndices.resize(4);  // None, Name, Date, Size

    // Search text
    connect(this, &Model::searchTextChanged, this, &Model::_onSearchTextChanged);
}

WallReel::Core::Image::Model::~Model() {
    m_watcher.cancel();
    m_watcher.waitForFinished();
    qDeleteAll(m_data);
}

int WallReel::Core::Image::Model::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_filteredIndices.count();
}

QVariant WallReel::Core::Image::Model::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_filteredIndices.count()) {
        Logger::debug("Invalid index requested: " + QString::number(index.row()));
        return QVariant();
    }

    int actualIndex = fromProxyIndex(index.row());
    if (actualIndex < 0 || actualIndex >= m_data.count()) {
        Logger::debug("Actual index out of bounds: " + QString::number(actualIndex));
        return QVariant();
    }
    // Logger::debug("Data requested for index: " + QString::number(index.row()) + ", actual index: " + QString::number(actualIndex) + ", role: " + QString::number(role));
    const auto& item = m_data[actualIndex];
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

QString WallReel::Core::Image::Model::currentSortType() const {
    switch (m_currentSortType) {
        case Config::SortType::None:
            return "None";
        case Config::SortType::Name:
            return "Name";
        case Config::SortType::Date:
            return "Date";
        case Config::SortType::Size:
            return "Size";
        default:
            return "Unknown";
    }
}

void WallReel::Core::Image::Model::setCurrentSortType(const QString& type) {
    Config::SortType newSortType = Config::SortType::None;
    if (type == "None") {
        newSortType = Config::SortType::None;
    } else if (type == "Name") {
        newSortType = Config::SortType::Name;
    } else if (type == "Date") {
        newSortType = Config::SortType::Date;
    } else if (type == "Size") {
        newSortType = Config::SortType::Size;
    }

    if (m_currentSortType != newSortType) {
        m_currentSortType = newSortType;
        emit currentSortTypeChanged();
    }
}

void WallReel::Core::Image::Model::setCurrentSortReverse(bool reverse) {
    if (m_currentSortReverse != reverse) {
        m_currentSortReverse = reverse;
        emit currentSortReverseChanged();
    }
}

void WallReel::Core::Image::Model::setSearchText(const QString& text) {
    // Logger::debug("Search text changed: " + text);
    if (m_searchText != text) {
        m_searchText = text;
        _applySearchFilter();
        emit searchTextChanged();
    }
}

const WallReel::Core::Image::Data* WallReel::Core::Image::Model::getDataPtrAt(int index) const {
    if (index < 0 || index >= m_filteredIndices.count()) {
        Logger::debug("Invalid index requested: " + QString::number(index));
        return nullptr;
    }
    int actualIndex = fromProxyIndex(index);
    if (actualIndex < 0 || actualIndex >= m_data.count()) {
        Logger::debug("Actual index out of bounds: " + QString::number(actualIndex));
        return nullptr;
    }
    return m_data[actualIndex];
}

QVariant WallReel::Core::Image::Model::dataAt(int index, const QString& roleName) const {
    if (index < 0 || index >= m_filteredIndices.count()) {
        Logger::debug("Invalid index requested: " + QString::number(index));
        return QVariant();
    }

    int actualIndex = fromProxyIndex(index);
    if (actualIndex < 0 || actualIndex >= m_data.count()) {
        Logger::debug("Actual index out of bounds: " + QString::number(actualIndex));
        return QVariant();
    }
    const auto& item = m_data[actualIndex];
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

void WallReel::Core::Image::Model::loadAndProcess(const QStringList& paths) {
    if (m_isLoading) {
        Logger::warn("Already loading images. Ignoring new load request.");
        return;
    }
    m_isLoading = true;
    emit isLoadingChanged();

    _clearData();

    m_processedCount = 0;
    m_progressUpdateTimer.start(s_ProgressUpdateIntervalMs);
    const auto thumbnailSize = m_thumbnailSize;
    const auto counterPtr    = &m_processedCount;
    QFuture<Data*> future    = QtConcurrent::mapped(paths, [thumbnailSize, counterPtr](const QString& path) {
        auto data = Data::create(path, thumbnailSize);
        counterPtr->fetch_add(1, std::memory_order_relaxed);
        return data;
    });
    m_watcher.setFuture(future);
    emit totalCountChanged();
}

int WallReel::Core::Image::Model::fromProxyIndex(int proxyIndex) const {
    if (proxyIndex < 0 || proxyIndex >= m_filteredIndices.size()) {
        Logger::debug("Invalid proxy index requested: " + QString::number(proxyIndex));
        return -1;
    }
    return m_filteredIndices[m_currentSortReverse ? (m_filteredIndices.size() - 1 - proxyIndex) : proxyIndex];
}

void WallReel::Core::Image::Model::focusOnIndex(int index) {
    if (index < 0 || index >= m_filteredIndices.count()) {
        Logger::debug("Invalid index to focus on: " + QString::number(index));
        return;
    }
    int actualIndex = fromProxyIndex(index);
    if (actualIndex < 0 || actualIndex >= m_data.count()) {
        Logger::debug("Actual index out of bounds for focus: " + QString::number(actualIndex));
        return;
    }
    m_focusedIndex = index;
    _updateFocusedName();
}

void WallReel::Core::Image::Model::stop() {
    if (m_isLoading) {
        Logger::info("Stopping image loading...");
        m_watcher.cancel();
    } else {
        Logger::warn("No loading operation to stop.");
    }
}

void WallReel::Core::Image::Model::_clearData() {
    beginResetModel();
    m_provider.clear();
    qDeleteAll(m_data);
    m_data.clear();
    for (auto& i : m_sortIndices) {
        i.clear();
    }
    m_filteredIndices.clear();
    endResetModel();
}

void WallReel::Core::Image::Model::_updateSortIndices(Config::SortType type) {
    QList<int>& indices = m_sortIndices[static_cast<int>(type)];
    indices.resize(m_data.count());
    std::iota(indices.begin(), indices.end(), 0);
    if (type == Config::SortType::None) {
        return;
    }

    const auto& compareFunc = [this, type](int idx1, int idx2) {
        const Data* data1 = m_data[idx1];
        const Data* data2 = m_data[idx2];
        if (!data1 || !data2) {
            return false;
        }
        switch (type) {
            case Config::SortType::Name:
                return QString::compare(data1->getFileName(), data2->getFileName(), Qt::CaseInsensitive) < 0;
            case Config::SortType::Date:
                return data1->getLastModified() < data2->getLastModified();
            case Config::SortType::Size:
                return data1->getSize() < data2->getSize();
            default:
                return false;
        }
    };

    std::sort(indices.begin(), indices.end(), compareFunc);
}

void WallReel::Core::Image::Model::_updateFocusedName() {
    if (m_focusedIndex < 0 || m_focusedIndex >= m_data.count()) {
        m_focusedName = "";
    } else {
        int actualIndex = fromProxyIndex(m_focusedIndex);
        if (actualIndex < 0 || actualIndex >= m_data.count()) {
            m_focusedName = "";
        } else {
            const auto& item = m_data[actualIndex];
            m_focusedName    = item->getFileName();
        }
    }
    emit focusedNameChanged();
}

void WallReel::Core::Image::Model::_applySearchFilter() {
    emit layoutAboutToBeChanged();
    m_filteredIndices.clear();
    const auto& sortedIndices = m_sortIndices[static_cast<int>(m_currentSortType)];
    for (int i = 0; i < sortedIndices.size(); ++i) {
        int actualIndex  = sortedIndices[i];
        const auto& item = m_data[actualIndex];
        if (item->getFileName().contains(m_searchText, Qt::CaseInsensitive)) {
            m_filteredIndices.append(actualIndex);
        }
    }
    emit layoutChanged();
}

void WallReel::Core::Image::Model::_onProgressValueChanged(int value) {
    Q_UNUSED(value);
    emit progressChanged();
}

void WallReel::Core::Image::Model::_onProcessingFinished() {
    auto results = m_watcher.future().results();

    beginResetModel();

    for (auto& data : results) {
        if (data && data->isValid()) {
            m_data.append(data);
            m_provider.insert(data);
        } else {
            Logger::warn("Failed to load image: " + (data ? data->getFullPath() : "null"));
            delete data;
            data = nullptr;
        }
    }

    for (int i = 0; i < m_sortIndices.size(); ++i) {
        _updateSortIndices(static_cast<Config::SortType>(i));
    }

    _applySearchFilter();

    endResetModel();

    Logger::info("Finished loading images. Total valid images: " + QString::number(m_data.count()));

    m_isLoading = false;
    m_progressUpdateTimer.stop();
    emit progressChanged();
    // emit isLoadingChanged();
    // QTimer::singleShot(s_IsLoadingUpdateIntervalMs, this, [this]() {
    //     emit isLoadingChanged();
    // });
    emit isLoadingChanged();
}

void WallReel::Core::Image::Model::_onSortMethodChanged() {
    _applySearchFilter();
}

void WallReel::Core::Image::Model::_onSearchTextChanged() {
    _updateFocusedName();
}
