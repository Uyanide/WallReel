#ifndef WALLREEL_IMAGEDATA_HPP
#define WALLREEL_IMAGEDATA_HPP

#include <QFileInfo>
#include <QImage>

class ImageData {
    QString id;
    QFileInfo file;
    QImage image;

    ImageData(const QString& path, const QSize& size);

  public:
    static ImageData* create(const QString& path, const QSize& size);

    const QImage& getImage() const { return image; }

    const QString& getId() const { return id; }

    bool isValid() const { return !image.isNull(); }

    QString getFullPath() const { return file.absoluteFilePath(); }

    QString getFileName() const { return file.fileName(); }

    QDateTime getLastModified() const { return file.lastModified(); }

    qint64 getSize() const { return file.size(); }

    const QFileInfo& getFileInfo() const { return file; }

  private:
};

#endif  // WALLREEL_IMAGEDATA_HPP
