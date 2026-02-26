#ifndef WALLREEL_IMAGEDATA_HPP
#define WALLREEL_IMAGEDATA_HPP

#include <QFileInfo>
#include <QImage>

namespace WallReel::Core::Image {

class Data {
    QString m_id;
    QFileInfo m_file;
    QImage m_image;
    QColor m_dominantColor;
    QHash<QString, QString> m_colorCache;

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

    const QColor& getDominantColor() const { return m_dominantColor; }

    std::optional<QString> getCachedColor(const QString& paletteName) const {
        if (m_colorCache.contains(paletteName)) {
            return m_colorCache.value(paletteName);
        }
        return std::nullopt;
    }

    void cacheColor(const QString& paletteName, const QString& colorName) {
        m_colorCache.insert(paletteName, colorName);
    }
};

}  // namespace WallReel::Core::Image

#endif  // WALLREEL_IMAGEDATA_HPP
