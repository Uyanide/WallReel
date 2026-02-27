#ifndef WALLREEL_IMAGEDATA_HPP
#define WALLREEL_IMAGEDATA_HPP

#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QUrl>

#include "Cache/manager.hpp"

// Development note
/*
Current implementation of image loading and caching:
1. Generate a unique ID for the image based on:
    - File path
    - Last modified timestamp
    - Target size (width x height)
   and use it as the cache key.
2. Check if a cached version of the image exists in the cache directory using the generated ID.
    - If so, load the image from the cache and construct the Data object accordingly.
    - If not:
        a. Load the original image from disk.
        b. Scale and crop it to the target size.
        c. Save the processed image to the cache directory using the generated ID as the filename.
        d. Construct the Data object with the new generated image.

Why this approach - Main purposes
- Fast decoding:
    By resizing and caching the image at the loading stage, the frontend can directly load the image
    at a smaller size and avoid the overhead of downsizing large (8K+ for example) images in memory,
    which can lead to significant performance improvements and reduced memory usage on the frontend.
- Memory efficiency:
    - Avoid keeping pixel data in memory for all images, and only load on demand by the frontend. Even
      keeping the resized image in memory can be costly if there are many, and the overhead of loading
      small images from disk is generally negligible and acceptable.
    - Resizing during loading fundamentally eliminates the possibility of the frontend storing large
      images in memory. (and not all image formats support `sourceSize` property in the right way)

*/

namespace WallReel::Core::Image {

/**
 * @brief A Model class representing an image file
 *
 */
class Data {
    Cache::Manager& m_cacheMgr;

    QString m_id;                          ///< Unique identifier for the image
    QFileInfo m_file;                      ///< File information of the image
    QFileInfo m_cachedFile;                ///< Cached file information for the loaded image
    QSize m_targetSize;                    ///< Target size for the loaded image
    QColor m_dominantColor;                ///< Dominant color of the image, used for palette matching
    QHash<QString, QString> m_colorCache;  ///< Cache for palette color matching results, key is palette name, value is matched color name

    Data(const QString& path, const QSize& size, Cache::Manager& cacheMgr);

  public:
    /**
     * @brief Factory method to create a Data instance from a file path. Returns nullptr if loading fails.
     *
     * @param path File path of the image
     * @param size Target size for loaded image, the image will be scaled and cropped to this size and stored in memory
     * @return Data*
     */
    static Data* create(const QString& path, const QSize& size, Cache::Manager& cacheMgr);

    QSize getTargetSize() const { return m_targetSize; }

    QString getId() const { return m_id; }

    QUrl getUrl() const { return QUrl::fromLocalFile(m_cachedFile.absoluteFilePath()); }

    bool isValid() const { return m_cachedFile.exists(); }

    QString getFullPath() const { return m_file.absoluteFilePath(); }

    QString getFileName() const { return m_file.fileName(); }

    QDateTime getLastModified() const { return m_file.lastModified(); }

    qint64 getSize() const { return m_file.size(); }

    const QFileInfo& getFileInfo() const { return m_file; }

    QImage loadImage() const;

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
