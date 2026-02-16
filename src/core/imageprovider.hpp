#ifndef WALLREEL_IMAGEPROVIDER_HPP
#define WALLREEL_IMAGEPROVIDER_HPP

#include <QHash>
#include <QMutex>
#include <QQuickImageProvider>

#include "imagedata.hpp"

class ImageProvider : public QQuickImageProvider {
    Q_OBJECT

  public:
    ImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

    void insert(ImageData* data);

    void clear();

  private:
    QMutex m_mutex;
    QHash<QString, ImageData*> m_images;
};

#endif  // WALLREEL_IMAGEPROVIDER_HPP
