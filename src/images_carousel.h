/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-05 01:22:53
 * @LastEditTime: 2026-01-15 07:06:46
 * @Description: Animated carousel widget for displaying and selecting images.
 */
#ifndef IMAGES_CAROUSEL_H
#define IMAGES_CAROUSEL_H

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMutex>
#include <QPointer>
#include <QPropertyAnimation>
#include <QQueue>
#include <QRunnable>
#include <QScrollArea>
#include <QTimer>

#include "config.h"
#include "image_item.h"

// Two different image loading strategies:
// - With loading screen: load all images directly
//   1. appendImages called -> increace m_addedImagesCount & spawn all ImageLoader
//      threads
//   2. Each ImageLoader calls _insertImageQueue with queued connection
//   3. _insertImageQueue calls _insertImage directly
// - Without loading screen: queue loaded images and insert them in batches
//   1. appendImages called -> increace m_addedImagesCount & spawn all ImageLoader
//      threads and a timer m_imageInsertQueueTimer, disable UI updates
//   2. Each ImageLoader calls _insertImageQueue with queued connection
//   3. _insertImageQueue enqueues the ImageData
//   4. m_imageInsertQueueTimer calls _processImageInsertQueue every
//      s_processBatchTimeout ms
//   5. _processImageInsertQueue processes up to s_processBatchSize items from the
//      queue and calls _insertImage for each
//
// The stop logic is identical:
// - Force stop
//   1. Set m_stopSign to true
//   2. ImageLoader::run checks m_stopSign and returns early if true
//      and calls _insertImageQueue using queued connection, but passing a
//      nullptr as parameter
//   3. The callstack from _insertImageQueue to _insertImage is same as above
//   4. _insertImage ignores nullptr and just increases m_processedImagesCount
//   5. when m_processedImagesCount >= m_addedImagesCount, emit stopped()
//   6. Call ImagesCarousel::_onImagesLoaded
// - Normal completion
//   1. Same as above until _insertImage, but m_stopSign is false and ImageLoader::run
//      passes valid ImageData pointer to _insertImageQueue
//   2. When m_processedImagesCount >= m_addedImagesCount, emit loadingCompleted()
//   3. Call ImagesCarousel::_onImagesLoaded
//
// 3 different ways to change focusing image:
// - focusNextImage / focusPrevImage: directly change m_currentIndex and call
//   focusCurrImage
//   These can be triggered by different events, e.g. key press, button click, etc.
// - Auto focus on scroll: debounce scroll events and calculate the nearest image
//   index to focus, then change m_currentIndex and call focusCurrImage
// - Initial focus: set m_currentIndex to 0 and call focusCurrImage
//
// Note:
// - All methods and slots of ImageCarousel should be called from the main thread.
// - ImageCarousel::m_addedImagesCount and ImageCarousel::m_processedImagesCount
//   should be identical after loading is finished, regardless of whether loading is
//   forcedly stopped or completed normally.
// - ImageCarousel::getLoadedImagesCount() returns the number of images currently
//   displayed in the carousel, which may be less than m_addedImagesCount if loading
//   is not yet completed or some images failed to load.
// - The current implementation actually supports dynamic addition of images during runtime,
//   but the UI does not provide such functionality yet and thus it is not tested :)

class ImageLoader;
class ImagesCarousel;
class ImagesCarouselScrollArea;

/**
 * @brief Worker class for loading images in a separate thread.
 */
class ImageLoader : public QRunnable {
  public:
    ImageLoader(const QString& path, ImagesCarousel* carousel);
    void run() override;  // friend to ImagesCarousel

  private:
    QString m_path;
    QPointer<ImagesCarousel> m_carousel;
    const int m_initWidth;
    const int m_initHeight;
};

namespace Ui {
class ImagesCarousel;
}

class ImagesCarousel : public QWidget {
    Q_OBJECT

    friend void ImageLoader::run();

  public:
    explicit ImagesCarousel(const Config::StyleConfigItems& styleConfig,
                            const Config::SortConfigItems& sortConfig,
                            QWidget* parent = nullptr);
    ~ImagesCarousel();

    static constexpr int s_debounceInterval  = 200;  // ms
    static constexpr int s_animationDuration = 300;  // ms

    static constexpr int s_processBatchTimeout = 50;  // ms
    static constexpr int s_processBatchSize    = 30;  // items

    /**
     * @brief Get the Current Image Path
     *
     * @return QString
     *
     * @note This method should be always called from the main thread.
     */
    [[nodiscard]] QString getCurrentImagePath() const {
        if (m_currentIndex >= 0 && m_currentIndex < getLoadedImagesCount()) {
            auto item = getImageItemAt(m_currentIndex);
            if (item) {
                return item->getFileFullPath();
            }
        }
        return "";
    }

    /**
     * @brief Get count of loaded images
     *
     * @return qsizetype
     *
     * @note This method should be always called from the main thread.
     */
    [[nodiscard]] qsizetype getLoadedImagesCount() const {
        return m_imagesLayout->count();
    }

    /**
     * @brief Get the Image object at index
     *
     * @param index
     * @return ImageItem*
     *
     * @note This method should be always called from the main thread.
     */
    [[nodiscard]] ImageItem* getImageItemAt(int index) const {
        if (index < 0 || index >= getLoadedImagesCount()) {
            return nullptr;
        }
        return dynamic_cast<ImageItem*>(
            m_imagesLayout
                ->itemAt(index)
                ->widget());
    }

    /**
     * @brief Get count of added images
     *
     * @return qsizetype
     */
    [[nodiscard]] qsizetype getAddedImagesCount() {
        QMutexLocker locker(&m_countMutex);
        return m_addedImagesCount;
    }

    // config items
    const int m_itemWidth;
    const int m_itemHeight;
    const int m_itemFocusWidth;
    const int m_itemFocusHeight;
    const Config::SortType m_sortType;
    const bool m_sortReverse;
    const bool m_noLoadingScreen;

  public slots:
    void focusNextImage();
    void focusPrevImage();
    void focusCurrImage();
    void unfocusCurrImage();
    void onStop();

  private slots:
    void _onScrollBarValueChanged(int value);
    void _onItemClicked(const QString& path);
    void _onImagesLoaded();  // Called when loading is completed or stopped
    void _processImageInsertQueue();

  public:
    void appendImages(const QStringList& paths);

  private:
    int _insertImage(ImageData* item);
    Q_INVOKABLE void _insertImageQueue(ImageData* item);

    void _enableUIUpdates(bool enable);
    int _focusingLeftOffset(int index);

  private:
    // UI elements
    Ui::ImagesCarousel* ui;
    QHBoxLayout* m_imagesLayout            = nullptr;
    ImagesCarouselScrollArea* m_scrollArea = nullptr;

    // Items and counters
    int m_processedImagesCount = 0;  // increase when _insertImage is called OR ImageLoader::run() is called with m_stopSign as true
    int m_addedImagesCount     = 0;  // increase when appendImages called
    QMutex m_countMutex;             // for m_processedImagesCount and m_addedImagesCount
    int m_currentIndex = -1;         // initially no focus

    // Threading
    QQueue<ImageData*> m_imageInsertQueue;
    QMutex m_imageInsertQueueMutex;
    QTimer* m_imageInsertQueueTimer = nullptr;

    // Animations
    QPropertyAnimation* m_scrollAnimation = nullptr;
    bool m_animationEnabled               = true;

    // Auto focusing
    bool m_suppressAutoFocus      = false;
    int m_pendingScrollValue      = 0;
    QTimer* m_scrollDebounceTimer = nullptr;

    // Loading stopped by user
    QMutex m_stopSignMutex;
    bool m_stopSign = false;

  signals:
    void imageFocused(const QString& path, const int index, const int count);

    void loadingStarted(const qsizetype amount);
    void loadingCompleted(const qsizetype amount);
    void imageLoaded(const qsizetype count);

    void stopped();
};

class ImagesCarouselScrollArea : public QScrollArea {
    Q_OBJECT

  public:
    explicit ImagesCarouselScrollArea(QWidget* parent = nullptr)
        : QScrollArea(parent) {
        // setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        // setWidgetResizable(true);
    }

    void setBlockInput(bool block) { m_blockInput = block; }

  protected:
    void keyPressEvent(QKeyEvent* event) override {
        if (m_blockInput) {
            event->ignore();
            return;
        }
        if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) {
            event->ignore();
        } else {
            QScrollArea::keyPressEvent(event);
        }
    }

    void wheelEvent(QWheelEvent* event) override {
        if (m_blockInput) {
            event->ignore();
            return;
        }
        if (event->angleDelta().y() != 0) {
            event->ignore();
        } else {
            QScrollArea::wheelEvent(event);
        }
    }

  private:
    bool m_blockInput = false;
};

#endif  // IMAGES_CAROUSEL_H
