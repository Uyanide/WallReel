#include "imagedata.hpp"

#include <QImageReader>

#include "utils/logger.hpp"

using namespace GeneralLogger;

ImageData* ImageData::create(const QString& path, const QSize& size) {
    ImageData* ret = new ImageData(path, size);
    if (!ret->isValid()) {
        delete ret;
        return nullptr;
    }
    return ret;
}

ImageData::ImageData(const QString& path, const QSize& targetSize)
    : file(path) {
    QImageReader reader(path);
    if (!reader.canRead()) {
        warn(QString("Failed to load image from path: %1").arg(path));
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

    if (!reader.read(&image)) {
        warn(QString("Failed to load image from path: %1").arg(path));
        return;
    }

    if (image.width() > processSize.width() || image.height() > processSize.height()) {
        image = image.scaled(processSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    // Crop to target size if necessary
    if (image.size() != targetSize) {
        int x = (image.width() - targetSize.width()) / 2;
        int y = (image.height() - targetSize.height()) / 2;
        image = image.copy(x, y, targetSize.width(), targetSize.height());
    }

    // Convert to GPU-friendly format
    if (image.format() != QImage::Format_ARGB32_Premultiplied) {
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    }

    // Create ID
    id = QString::number(qHash(file.absoluteFilePath()));
}
