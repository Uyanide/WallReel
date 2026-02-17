
#include "provider.hpp"

QImage WallReel::Core::Image::Provider::requestImage(const QString& id, QSize* size, const QSize& requestedSize) {
    QMutexLocker locker(&m_mutex);
    if (!m_images.contains(id)) {
        return QImage();
    }
    Data* data = m_images[id];
    if (size) {
        *size = data->getImage().size();
    }
    return data->getImage();
}

void WallReel::Core::Image::Provider::insert(Data* data) {
    QMutexLocker locker(&m_mutex);
    m_images.insert(data->getId(), data);
}

void WallReel::Core::Image::Provider::clear() {
    QMutexLocker locker(&m_mutex);
    m_images.clear();
}
