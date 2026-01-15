/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-11-30 20:31:15
 * @LastEditTime: 2026-01-15 07:10:21
 * @Description: Image item widget for displaying an image.
 */
#ifndef IMAGE_ITEM_H
#define IMAGE_ITEM_H

#include <QDateTime>
#include <QFileInfo>
#include <QImage>
#include <QLabel>
#include <QPropertyAnimation>

class ImageData;
class ImageItem;

/**
 * @brief Data structure to hold image information
 *        and can be safely created and passed between threads.
 */
class ImageData {
    QFileInfo file;
    QImage* image;

  public:
    static ImageData* create(const QString& p, const int initWidth, const int initHeight);

    ~ImageData() { releaseImage(); }

    // Optimization: release image data as soon as they are no longer needed
    void releaseImage() { delete image, image = nullptr; }

    [[nodiscard]] const QImage& getImage() const { return *image; }

    [[nodiscard]] bool isValid() const { return !image->isNull(); }

    [[nodiscard]] const QFileInfo& getFileInfo() const { return file; }

  private:
    ImageData(const QString& path) : file(path), image() {}
};

/**
 * @brief Image label that displays an image,
 *        which should always be created in the main thread.
 */
class ImageItem : public QLabel {
    Q_OBJECT

  public:
    static constexpr int s_animationDuration = 300;

    explicit ImageItem(ImageData* data,
                       const int itemWidth,
                       const int itemHeight,
                       const int itemFocusWidth,
                       const int itemFocusHeight,
                       QWidget* parent = nullptr);

    ~ImageItem() override;

    [[nodiscard]] QString getFileFullPath() const { return m_data->getFileInfo().absoluteFilePath(); }

    [[nodiscard]] QString getFileName() const { return m_data->getFileInfo().fileName(); }

    [[nodiscard]] QDateTime getFileDate() const { return m_data->getFileInfo().lastModified(); }

    [[nodiscard]] const QImage& getThumbnail() const { return m_data->getImage(); }

    [[nodiscard]] qint64 getFileSize() const { return m_data->getFileInfo().size(); }

    /**
     * @brief Set focus state by scaling the image label
     *
     * @param focus whether to focus
     * @param animate whether to animate the transition
     */
    void setFocus(bool focus = true, bool animate = true);

  protected:
    void mousePressEvent(QMouseEvent* event) override {
        emit clicked(getFileFullPath());
        QLabel::mousePressEvent(event);
    }

  private:
    const ImageData* m_data;
    QSize m_itemSize;
    QSize m_itemFocusSize;
    QPropertyAnimation* m_scaleAnimation = nullptr;

  signals:
    void clicked(const QString& path);
};

#endif  // IMAGE_ITEM_H
