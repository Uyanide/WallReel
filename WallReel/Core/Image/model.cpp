#include "model.hpp"

#include "logger.hpp"

WALLREEL_DECLARE_SENDER("ImageModel")

namespace WallReel::Core::Image {

Model::Model(QObject* parent) : QAbstractListModel(parent) {}

Model::~Model() {
    qDeleteAll(m_data);
    m_data.clear();
}

int Model::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_data.count();
}

QVariant Model::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_data.count() || index.row() < 0) {
        WR_DEBUG("Invalid index requested: " + QString::number(index.row()));
        return QVariant();
    }

    const auto& item = m_data[index.row()];
    switch (role) {
        case IdRole:
            return item->getId();
        case UrlRole:
            return item->getUrl();
        case PathRole:
            return item->getFullPath();
        case NameRole:
            return item->getFileName();
        case SizeRole:
            return item->getSize();
        case DateRole:
            return item->getLastModified();
        case DomColorRole:
            return item->getDominantColor();
        default:
            return QVariant();
    }
}

QVariant Model::dataAt(int index, const QString& roleName) const {
    if (index < 0 || index >= m_data.count()) {
        WR_DEBUG("Invalid index requested: " + QString::number(index));
        return QVariant();
    }

    const auto& item = m_data[index];
    if (roleName == "imgId") {
        return item->getId();
    } else if (roleName == "imgUrl") {
        return item->getUrl();
    } else if (roleName == "imgPath") {
        return item->getFullPath();
    } else if (roleName == "imgName") {
        return item->getFileName();
    } else if (roleName == "imgSize") {
        return item->getSize();
    } else if (roleName == "imgDate") {
        return item->getLastModified();
    } else if (roleName == "imgDomColor") {
        return item->getDominantColor();
    } else {
        return QVariant();
    }
}

void Model::insertData(const QList<Data*>& newData) {
    if (newData.isEmpty()) {
        return;
    }
    beginInsertRows(QModelIndex(), m_data.count(), m_data.count() + newData.count() - 1);
    m_data.append(newData);
    endInsertRows();
}

void Model::clearData() {
    if (m_data.isEmpty()) {
        return;
    }
    beginResetModel();
    qDeleteAll(m_data);
    m_data.clear();
    endResetModel();
}

}  // namespace WallReel::Core::Image
