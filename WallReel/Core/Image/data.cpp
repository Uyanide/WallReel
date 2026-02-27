#include "data.hpp"

#include <QCryptographicHash>
#include <QImageReader>

#include "Palette/domcolor.hpp"
#include "logger.hpp"

WALLREEL_DECLARE_SENDER("ImageData")

WallReel::Core::Image::Data* WallReel::Core::Image::Data::create(
    const QString& path,
    const QSize& size,
    Cache::Manager& cacheMgr) {
    Data* ret = new Data(path, size, cacheMgr);
    if (!ret->isValid()) {
        delete ret;
        return nullptr;
    }
    return ret;
}

WallReel::Core::Image::Data::Data(const QString& path, const QSize& targetSize, Cache::Manager& cacheMgr)
    : m_cacheMgr(cacheMgr), m_file(path), m_targetSize(targetSize) {
    m_id            = cacheMgr.cacheKey(m_file, m_targetSize);
    m_cachedFile    = cacheMgr.getImage(m_id, [this]() { return computeImage(); });
    m_dominantColor = cacheMgr.getColor(m_id, [this]() { return computeDominantColor(loadImage()); });
}

QImage WallReel::Core::Image::Data::loadImage() const {
    QImageReader reader(m_cachedFile.absoluteFilePath());
    if (!reader.canRead()) {
        WR_WARN("Cannot read cached image: " + m_cachedFile.absoluteFilePath());
        return QImage();
    }

    QImage image;
    if (!reader.read(&image)) {
        WR_WARN("Failed to read cached image: " + m_cachedFile.absoluteFilePath());
        return QImage();
    }
    return image;
}

QImage WallReel::Core::Image::Data::computeImage() const {
    QImageReader reader(m_file.absoluteFilePath());
    if (!reader.canRead()) {
        WR_WARN("Cannot read image file: " + m_file.absoluteFilePath());
        return QImage();
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
        WR_WARN("Failed to read image file: " + m_file.absoluteFilePath());
        return QImage();
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
    return image;
}

QColor WallReel::Core::Image::Data::computeDominantColor(const QImage& image) const {
    return Palette::getDominantColor(image);
}
