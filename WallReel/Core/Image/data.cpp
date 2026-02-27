#include "data.hpp"

#include <QCryptographicHash>
#include <QImageReader>

#include "Palette/domcolor.hpp"
#include "logger.hpp"

WallReel::Core::Image::Data* WallReel::Core::Image::Data::create(
    const QString& path,
    const QSize& size,
    const QDir& cacheDir) {
    Data* ret = new Data(path, size, cacheDir);
    if (!ret->isValid()) {
        delete ret;
        return nullptr;
    }
    return ret;
}

WallReel::Core::Image::Data::Data(const QString& path, const QSize& targetSize, const QDir& cacheDir)
    : m_file(path), m_targetSize(targetSize) {
    m_id                 = _generateId(path, targetSize);
    const auto cachePath = cacheDir.absoluteFilePath(_generateCacheFileName(m_id));
    m_cachedFile         = QFileInfo(cachePath);

    // If cached file exists, use it directly
    if (m_cachedFile.exists()) {
        Logger::debug(QString("Cache hit for image: %1").arg(m_file.absoluteFilePath()));
        if (!_loadFromCache()) {
            Logger::warn(QString("Failed to load cached image from path: %1").arg(m_cachedFile.absoluteFilePath()));
            if (!_loadFresh()) {
                Logger::warn(QString("Failed to load fresh image from path: %1").arg(m_file.absoluteFilePath()));
            }
        }
    } else {
        if (!_loadFresh()) {
            Logger::warn(QString("Failed to load fresh image from path: %1").arg(m_file.absoluteFilePath()));
        }
    }
}

bool WallReel::Core::Image::Data::_loadFresh() {
    QImageReader reader(m_file.absoluteFilePath());
    if (!reader.canRead()) {
        return false;
    }

    const QSize originalSize = reader.size();

    // Scale the image to fit the target size while maintaining aspect ratio
    QSize processSize = originalSize;
    if (originalSize.isValid()) {
        double widthRatio  = (double)m_targetSize.width() / originalSize.width();
        double heightRatio = (double)m_targetSize.height() / originalSize.height();
        double scaleFactor = std::max(widthRatio, heightRatio);
        processSize        = originalSize * scaleFactor;

        // Use reader's built-in scaling if supported
        if (reader.supportsOption(QImageIOHandler::ScaledSize)) {
            reader.setScaledSize(processSize);
        }
    }

    QImage image;
    if (!reader.read(&image)) {
        return false;
    }

    // If reader doesn't support built-in scaling or the image still do not match the target size, do manual scaling
    if (image.size() != processSize) {
        image = image.scaled(processSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // Crop to target size if necessary
    if (image.size() != m_targetSize) {
        int x = (image.width() - m_targetSize.width()) / 2;
        int y = (image.height() - m_targetSize.height()) / 2;
        image = image.copy(x, y, m_targetSize.width(), m_targetSize.height());
    }

    // Convert to GPU-friendly format
    if (image.format() != QImage::Format_ARGB32_Premultiplied) {
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    // Get dominant color
    m_dominantColor = Palette::getDominantColor(image);

    // Save to cache
    if (!image.save(m_cachedFile.absoluteFilePath())) {
        Logger::warn(QString("Failed to save cached image to path: %1").arg(m_cachedFile.absoluteFilePath()));
        return false;
    } else {
        Logger::debug(QString("Cached image saved to path: %1").arg(m_cachedFile.absoluteFilePath()));
    }
    return true;
}

bool WallReel::Core::Image::Data::_loadFromCache() {
    QImageReader reader(m_cachedFile.absoluteFilePath());
    if (!reader.canRead()) {
        return false;
    }

    QImage image;
    if (!reader.read(&image)) {
        return false;
    }

    // Get dominant color
    m_dominantColor = Palette::getDominantColor(image);
    return true;
}

QImage WallReel::Core::Image::Data::loadImage() const {
    QImageReader reader(m_cachedFile.absoluteFilePath());
    if (!reader.canRead()) {
        return QImage();
    }

    QImage image;
    if (!reader.read(&image)) {
        return QImage();
    }
    return image;
}

QString WallReel::Core::Image::Data::_generateId(const QString& path, const QSize& size) {
    auto info = QFileInfo(path);
    auto key  = QString("%1|%2|%3|%4x%5").arg(info.absoluteFilePath()).arg(info.lastModified().toSecsSinceEpoch()).arg(info.size()).arg(size.width()).arg(size.height());

    QByteArray hash = QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Md5);
    return QString::fromLatin1(hash.toHex());
}

QString WallReel::Core::Image::Data::_generateCacheFileName(const QString& id) {
    return id + ".png";
}
