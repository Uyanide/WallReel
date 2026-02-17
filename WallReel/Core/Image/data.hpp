#ifndef WALLREEL_IMAGEDATA_HPP
#define WALLREEL_IMAGEDATA_HPP

#include <QFileInfo>
#include <QImage>

namespace WallReel::Core::Image {

class Data {
    QString m_id;
    QFileInfo m_file;
    QImage m_image;

    Data(const QString& path, const QSize& size);

  public:
    static Data* create(const QString& path, const QSize& size);

    const QImage& getImage() const { return m_image; }

    const QString& getId() const { return m_id; }

    bool isValid() const { return !m_image.isNull(); }

    QString getFullPath() const { return m_file.absoluteFilePath(); }

    QString getFileName() const { return m_file.fileName(); }

    QDateTime getLastModified() const { return m_file.lastModified(); }

    qint64 getSize() const { return m_file.size(); }

    const QFileInfo& getFileInfo() const { return m_file; }

  private:
};

}  // namespace WallReel::Core::Image

#endif  // WALLREEL_IMAGEDATA_HPP
