#include "data.hpp"

#include <QImageReader>

#include "Palette/domcolor.hpp"
#include "logger.hpp"

WallReel::Core::Image::Data* WallReel::Core::Image::Data::create(const QString& path, const QSize& size) {
    Data* ret = new Data(path, size);
    if (!ret->isValid()) {
        delete ret;
        return nullptr;
    }
    return ret;
}

WallReel::Core::Image::Data::Data(const QString& path, const QSize& targetSize)
    : m_file(path) {
    QImageReader reader(path);
    if (!reader.canRead()) {
        Logger::warn(QString("Failed to load image from path: %1").arg(path));
        return;
    }

    const QSize originalSize = reader.size();

    // Scale the image to fit the target size while maintaining aspect ratio
    QSize processSize = originalSize;
    if (originalSize.isValid()) {
        double widthRatio  = (double)targetSize.width() / originalSize.width();
        double heightRatio = (double)targetSize.height() / originalSize.height();
        double scaleFactor = std::max(widthRatio, heightRatio);
        processSize        = originalSize * scaleFactor;

        if (reader.supportsOption(QImageIOHandler::ScaledSize)) {
            reader.setScaledSize(processSize);
        }
    }

    if (!reader.read(&m_image)) {
        Logger::warn(QString("Failed to load image from path: %1").arg(path));
        return;
    }

    if (m_image.width() > processSize.width() || m_image.height() > processSize.height()) {
        m_image = m_image.scaled(processSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // Crop to target size if necessary
    if (m_image.size() != targetSize) {
        int x   = (m_image.width() - targetSize.width()) / 2;
        int y   = (m_image.height() - targetSize.height()) / 2;
        m_image = m_image.copy(x, y, targetSize.width(), targetSize.height());
    }

    // Convert to GPU-friendly format
    if (m_image.format() != QImage::Format_ARGB32_Premultiplied) {
        m_image = m_image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    // Create ID
    m_id = QString::number(qHash(m_file.absoluteFilePath()));

    // Get dominant color
    m_dominantColor = Palette::getDominantColor(m_image);
}
