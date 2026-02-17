#ifndef WALLREEL_IMAGEPROVIDER_HPP
#define WALLREEL_IMAGEPROVIDER_HPP

#include <QHash>
#include <QMutex>
#include <QQuickImageProvider>

#include "data.hpp"

namespace WallReel::Core::Image {

class Provider : public QQuickImageProvider {
    Q_OBJECT

  public:
    Provider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

    void insert(Data* data);

    void clear();

  private:
    QMutex m_mutex;
    QHash<QString, Data*> m_images;
};

}  // namespace WallReel::Core::Image

#endif  // WALLREEL_IMAGEPROVIDER_HPP
