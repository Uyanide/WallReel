/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-05 01:22:53
 * @LastEditTime: 2025-12-01 00:59:39
 * @Description: Animated carousel widget for displaying and selecting images.
 */
#ifndef IMAGES_CAROUSEL_H
#define IMAGES_CAROUSEL_H

#include <qqueue.h>
#include <qtmetamacros.h>

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QPointer>
#include <QPropertyAnimation>
#include <QQueue>
#include <QRunnable>
#include <QScrollArea>
#include <QThreadPool>
#include <QTimer>
#include <QWidget>

#include "config.h"
#include "image_item.h"

class ImageData;
class ImageItem;
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

    static constexpr int s_debounceInterval    = 200;
    static constexpr int s_animationDuration   = 300;
    static constexpr int s_processBatchTimeout = 50;  // ms
    static constexpr int s_processBatchSize    = 30;  // items

    [[nodiscard]] QString getCurrentImagePath() const {
        if (m_currentIndex < 0 || m_currentIndex >= m_loadedImages.size()) {
            return "";
        }
        return m_loadedImages[m_currentIndex]->getFileFullPath();
    }

    // Should always be called in the main thread
    [[nodiscard]] qsizetype getLoadedImagesCount() {
        return m_loadedImages.size();
    }

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
    void _onImagesLoaded();

    void _processImageInsertQueue();

  public:
    void appendImages(const QStringList& paths);

  private:
    int _insertImage(const ImageData* item);
    Q_INVOKABLE void _insertImageQueue(const ImageData* item);

    void _enableUIUpdates(bool enable);
    int _focusingLeftOffset(int index);

  private:
    // UI elements
    Ui::ImagesCarousel* ui;
    QHBoxLayout* m_imagesLayout            = nullptr;
    ImagesCarouselScrollArea* m_scrollArea = nullptr;

    // Items and counters
    QVector<ImageItem*> m_loadedImages;  // m_loadedImages.size() may != m_loadedImagesCount
    int m_loadedImagesCount = 0;         // increase when _insertImage is called OR ImageLoader::run() is called with m_stopSign as true
    int m_addedImagesCount  = 0;         // increase when appendImages called
    QMutex m_countMutex;                 // for m_loadedImagesCount and m_addedImagesCount
    int m_currentIndex = -1;             // initially no focus

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

    // Flags
    bool m_initialImagesLoaded = true;

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
