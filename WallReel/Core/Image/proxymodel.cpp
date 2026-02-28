#include "model.hpp"

namespace WallReel::Core::Image {

ProxyModel::ProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent) {
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(true);
}

void ProxyModel::setSearchText(const QString& text) {
    if (m_searchText != text) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
        beginFilterChange();
#endif
        m_searchText = text;
        setFilterFixedString(text);
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
        endFilterChange();
#else
        invalidateFilter();
#endif
    }
}

void ProxyModel::setSortType(Config::SortType type) {
    if (m_sortType != type) {
        m_sortType = type;
        invalidate();
        sort(0, m_sortDescending ? Qt::DescendingOrder : Qt::AscendingOrder);
    }
}

void ProxyModel::setSortDescending(bool descending) {
    if (m_sortDescending != descending) {
        m_sortDescending = descending;
        sort(0, m_sortDescending ? Qt::DescendingOrder : Qt::AscendingOrder);
    }
}

bool ProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    QString imageName = sourceModel()->data(index, Model::NameRole).toString();

    if (m_searchText.isEmpty()) return true;
    return imageName.contains(m_searchText, Qt::CaseInsensitive);
}

bool ProxyModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const {
    switch (m_sortType) {
        case Config::SortType::Name: {
            QString leftName  = sourceModel()->data(source_left, Model::NameRole).toString();
            QString rightName = sourceModel()->data(source_right, Model::NameRole).toString();
            return leftName < rightName;
        }
        case Config::SortType::Date: {
            QDateTime leftDate  = sourceModel()->data(source_left, Model::DateRole).toDateTime();
            QDateTime rightDate = sourceModel()->data(source_right, Model::DateRole).toDateTime();
            return leftDate < rightDate;
        }
        case Config::SortType::Size: {
            qint64 leftSize  = sourceModel()->data(source_left, Model::SizeRole).toLongLong();
            qint64 rightSize = sourceModel()->data(source_right, Model::SizeRole).toLongLong();
            return leftSize < rightSize;
        }
        default:
            qDebug() << "Unknown sort type:" << static_cast<int>(m_sortType);
            return false;
    }
}

}  // namespace WallReel::Core::Image
