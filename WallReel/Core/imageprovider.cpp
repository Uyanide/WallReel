
#include "imageprovider.hpp"

QImage ImageProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize) {
    QMutexLocker locker(&m_mutex);
    if (!m_images.contains(id)) {
        return QImage();
    }
    ImageData* data = m_images[id];
    if (size) {
        *size = data->getImage().size();
    }
    return data->getImage();
}

void ImageProvider::insert(ImageData* data) {
    QMutexLocker locker(&m_mutex);
    m_images.insert(data->getId(), data);
}

void ImageProvider::clear() {
    QMutexLocker locker(&m_mutex);
    m_images.clear();
}
