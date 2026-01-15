/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-11-30 20:32:27
 * @LastEditTime: 2026-01-15 00:48:24
 * @Description: Image item widget for displaying an image.
 */
#include "image_item.h"

#include <QImageReader>

#include "logger.h"

using namespace GeneralLogger;

ImageData* ImageData::create(const QString& p, const int initWidth, const int initHeight) {
    ImageData* data = new ImageData(p);
    data->image     = new QImage();

    QImageReader reader(p);
    if (!reader.canRead()) {
        warn(QString("Failed to load image from path: %1").arg(p));
        delete data;
        return nullptr;
    }

    const QSize targetSize(initWidth, initHeight);
    const QSize originalSize = reader.size();

    if (originalSize.isValid()) {
        double widthRatio  = (double)targetSize.width() / originalSize.width();
        double heightRatio = (double)targetSize.height() / originalSize.height();
        double scaleFactor = std::max(widthRatio, heightRatio);

        QSize scaledSize = originalSize * scaleFactor;
        reader.setScaledSize(scaledSize);
    }

    if (!reader.read(data->image)) {
        warn(QString("Failed to load image from path: %1").arg(p));
        delete data;
        return nullptr;
    }

    if (data->image->size() != targetSize) {
        int x        = (data->image->width() - targetSize.width()) / 2;
        int y        = (data->image->height() - targetSize.height()) / 2;
        *data->image = data->image->copy(x, y, targetSize.width(), targetSize.height());
    }

    return data;
}

ImageItem::ImageItem(ImageData* data,
                     const int itemWidth,
                     const int itemHeight,
                     const int itemFocusWidth,
                     const int itemFocusHeight,
                     QWidget* parent)
    : QLabel(parent),
      m_data(data),
      m_itemSize(itemWidth, itemHeight),
      m_itemFocusSize(itemFocusWidth, itemFocusHeight) {
    assert(data != nullptr);
    setScaledContents(true);
    if (!data->isValid()) {
        setText(":(");
        setAlignment(Qt::AlignCenter);
    } else {
        setPixmap(QPixmap::fromImage(data->getImage()));
        data->releaseImage();
    }
    setFixedSize(itemWidth, itemHeight);
}

ImageItem::~ImageItem() {
    if (m_scaleAnimation) {
        m_scaleAnimation->stop();
        delete m_scaleAnimation;
        m_scaleAnimation = nullptr;
    }
    delete m_data;
}

void ImageItem::setFocus(bool focus, bool animate) {
    if (!animate) {
        setFixedSize(focus ? m_itemFocusSize : m_itemSize);
        return;
    }
    if (m_scaleAnimation) {
        m_scaleAnimation->stop();
        delete m_scaleAnimation;
        m_scaleAnimation = nullptr;
    }
    m_scaleAnimation = new QPropertyAnimation(this, "size");
    m_scaleAnimation->setDuration(ImageItem::s_animationDuration);
    m_scaleAnimation->setStartValue(size());
    m_scaleAnimation->setEndValue(focus ? m_itemFocusSize : m_itemSize);
    m_scaleAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_scaleAnimation,
            &QPropertyAnimation::valueChanged,
            this,
            [this](const QVariant& value) {
                setFixedSize(value.toSize());
            });
    m_scaleAnimation->start();
}
