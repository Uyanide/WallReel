/*
 * @Author: Uyanide pywang0608@foxmail.com
 * @Date: 2025-08-05 01:22:53
 * @LastEditTime: 2026-01-14 23:50:49
 * @Description: Animated carousel widget for displaying and selecting images.
 */
#include "images_carousel.h"

#include <assert.h>
#include <pthread.h>
// #include <stdlib.h>

#include <QLabel>
#include <QMetaObject>
#include <QScrollArea>
#include <QScrollBar>
#include <QThreadPool>
#include <QVector>
#include <functional>

#include "image_item.h"
#include "logger.h"
#include "ui_images_carousel.h"
#include "utils.h"

using namespace GeneralLogger;

ImagesCarousel::ImagesCarousel(const Config::StyleConfigItems& styleConfig,
                               const Config::SortConfigItems& sortConfig,
                               QWidget* parent)
    : QWidget(parent),
      ui(new Ui::ImagesCarousel),
      m_itemWidth(styleConfig.imageWidth),
      m_itemHeight(static_cast<int>(m_itemWidth / styleConfig.aspectRatio)),
      m_itemFocusWidth(styleConfig.imageFocusWidth),
      m_itemFocusHeight(static_cast<int>(styleConfig.imageFocusWidth / styleConfig.aspectRatio)),
      m_sortType(sortConfig.type),
      m_sortReverse(sortConfig.reverse),
      m_noLoadingScreen(styleConfig.noLoadingScreen) {
    ui->setupUi(this);
    m_scrollArea   = dynamic_cast<ImagesCarouselScrollArea*>(ui->scrollArea);
    m_imagesLayout = dynamic_cast<QHBoxLayout*>(ui->scrollAreaWidgetContents->layout());

    // Remove border
    ui->scrollArea->setFrameShape(QFrame::NoFrame);

    connect(this,
            &ImagesCarousel::loadingCompleted,
            this,
            &ImagesCarousel::_onImagesLoaded);

    connect(this,
            &ImagesCarousel::stopped,
            this,
            &ImagesCarousel::_onImagesLoaded);

    // Auto focus when scrolling
    m_scrollDebounceTimer = new QTimer(this);
    m_scrollDebounceTimer->setSingleShot(true);
    m_scrollDebounceTimer->setInterval(s_debounceInterval);
    connect(m_scrollDebounceTimer,
            &QTimer::timeout,
            this,
            [this]() {
                _onScrollBarValueChanged(m_pendingScrollValue);
            });
    connect(ui->scrollArea->horizontalScrollBar(),
            &QScrollBar::valueChanged,
            this,
            [this](int value) {
                m_pendingScrollValue = value;
                if (m_suppressAutoFocus) {
                    return;
                }
                m_scrollDebounceTimer->start();
            });
}

void ImagesCarousel::_onImagesLoaded() {
    m_animationEnabled = true;
    if (!m_noLoadingScreen) {
        _enableUIUpdates(true);
    } else if (m_imageInsertQueueTimer) {
        m_imageInsertQueueTimer->stop();
        m_imageInsertQueueTimer->deleteLater();
        m_imageInsertQueueTimer = nullptr;
    }
    if (m_initialImagesLoaded) {
        // No images loaded
        if (!getLoadedImagesCount()) {
            return;
        }
        // Focus the first image
        if (m_currentIndex < 0) {
            m_currentIndex = 0;
            // Ensure the layout events are processed
            QTimer::singleShot(0, this, [this]() {
                focusCurrImage();
            });
        }
    }

    // exit(1);  // for debug
}

ImagesCarousel::~ImagesCarousel() {
    delete ui;
    if (m_scrollAnimation) {
        m_scrollAnimation->stop();
        m_scrollAnimation->deleteLater();
    }
}

void ImagesCarousel::appendImages(const QStringList& paths) {
    if (paths.isEmpty()) {
        warn("No images to add to display.");
        emit loadingCompleted(0);
        return;
    }
    {
        QMutexLocker locker(&m_countMutex);
        m_addedImagesCount += paths.size();
    }
    m_animationEnabled = false;
    if (!m_noLoadingScreen) {
        _enableUIUpdates(false);
    } else if (m_imageInsertQueueTimer == nullptr) {
        m_imageInsertQueueTimer = new QTimer(this);
        m_imageInsertQueueTimer->setInterval(s_processBatchTimeout);
        connect(m_imageInsertQueueTimer,
                &QTimer::timeout,
                this,
                &ImagesCarousel::_processImageInsertQueue);
        m_imageInsertQueueTimer->start();
    }
    emit loadingStarted(paths.size());
    for (const QString& path : paths) {
        ImageLoader* loader = new ImageLoader(path, this);
        QThreadPool::globalInstance()->start(loader);
    }
}

ImageLoader::ImageLoader(const QString& path, ImagesCarousel* carousel)
    : m_path(path),
      m_carousel(carousel),
      m_initWidth(carousel->m_itemFocusWidth),
      m_initHeight(carousel->m_itemFocusHeight) {
    setAutoDelete(true);
}

void ImagesCarousel::_insertImageQueue(ImageData* data) {
    if (!m_noLoadingScreen) {
        _insertImage(data);
        return;
    }
    {
        QMutexLocker locker(&m_imageInsertQueueMutex);
        m_imageInsertQueue.enqueue(const_cast<ImageData*>(data));
    }
}

int ImagesCarousel::_insertImage(ImageData* data) {
    // Increase loaded count regardless of success or failure
    Defer defer([this]() {
        emit imageLoaded(getLoadedImagesCount());
        {
            QMutexLocker countLocker(&m_countMutex);
            if (++m_loadedImagesCount >= m_addedImagesCount) {
                QMutexLocker stopSignLocker(&m_stopSignMutex);
                if (m_stopSign) {
                    // if all stopped
                    emit stopped();
                } else {
                    emit loadingCompleted(m_loadedImagesCount);
                }
            }
        }
        return;
    });

    if (!data) return -1;

    auto item = new ImageItem(
        data,
        m_itemWidth,
        m_itemHeight,
        m_itemFocusWidth,
        m_itemFocusHeight,
        this);

    static const QVector<std::function<bool(const ImageItem*, const ImageItem*)>> cmpFuncs = {
        [](auto, auto) {
            return false;
        },  // None
        [](auto a, auto b) {
            return a->getFileName() < b->getFileName();
        },
        [](auto a, auto b) {
            return a->getFileDate() < b->getFileDate();
        },
        [](auto a, auto b) {
            return a->getFileSize() < b->getFileSize();
        },
    };

    // insert into correct position based on sort type and direction
    qint64 insertPos = getLoadedImagesCount();
    if (m_sortType != Config::SortType::None) {
        auto cmp     = cmpFuncs[static_cast<int>(m_sortType)];
        auto reverse = m_sortReverse;

        int left = 0, right = getLoadedImagesCount();
        while (left < right) {
            int mid = left + (right - left) / 2;
            if (reverse ? cmp(getImageItemAt(mid), item) : cmp(item, getImageItemAt(mid))) {
                right = mid;
            } else {
                left = mid + 1;
            }
        }
        insertPos = left;
    }
    connect(item,
            &ImageItem::clicked,
            this,
            &ImagesCarousel::_onItemClicked);
    m_imagesLayout->insertWidget(insertPos, item);
    return insertPos;
}

void ImagesCarousel::_processImageInsertQueue() {
    QVector<ImageData*> batch;
    {
        QMutexLocker locker(&m_imageInsertQueueMutex);
        while (!m_imageInsertQueue.isEmpty() && batch.size() < s_processBatchSize) {
            batch.append(m_imageInsertQueue.dequeue());
        }
    }
    if (m_noLoadingScreen) _enableUIUpdates(false);
    int currPos = m_currentIndex;
    for (ImageData* data : batch) {
        int pos = _insertImage(data);
        if (pos >= 0 && pos <= currPos) {
            currPos++;
        }
    }
    // Update focusing index if any
    if (m_currentIndex >= 0) {
        m_currentIndex = currPos;
        if (m_currentIndex < 0 || m_currentIndex >= getLoadedImagesCount()) {
            m_currentIndex = 0;
        }
    }
    if (m_noLoadingScreen) _enableUIUpdates(true);
    focusCurrImage();
}

void ImagesCarousel::_enableUIUpdates(bool enable) {
    m_imagesLayout->setEnabled(enable);
    if (enable) {
        m_imagesLayout->activate();
    }
    ui->scrollAreaWidgetContents->setUpdatesEnabled(enable);
}

void ImageLoader::run() {
    ImageData* data = nullptr;
    Defer defer([this, &data]() {
        if (m_carousel.isNull()) {
            delete data;
            return;
        }
        QMetaObject::invokeMethod(m_carousel,
                                  "_insertImageQueue",
                                  Qt::QueuedConnection,
                                  Q_ARG(ImageData*, data));
    });
    {
        QMutexLocker stopSignLocker(&m_carousel->m_stopSignMutex);
        if (m_carousel->m_stopSign) return;
    }
    data = ImageData::create(m_path, m_initWidth, m_initHeight);
}

void ImagesCarousel::focusNextImage() {
    const auto count = getLoadedImagesCount();
    // If no focus, focus the first image
    if (m_currentIndex < 0) {
        if (!count) return;
        m_currentIndex = 0;
        focusCurrImage();
        return;
    }
    unfocusCurrImage();
    if (count <= 1) return;
    m_currentIndex++;
    if (m_currentIndex >= count) {
        m_currentIndex = 0;
    }
    focusCurrImage();
}

void ImagesCarousel::focusPrevImage() {
    const auto count = getLoadedImagesCount();
    // If no focus, focus the last image
    if (m_currentIndex < 0) {
        if (!count) return;
        m_currentIndex = count - 1;
        focusCurrImage();
        return;
    }
    if (count <= 1) return;
    unfocusCurrImage();
    m_currentIndex--;
    if (m_currentIndex < 0) {
        m_currentIndex = count - 1;
    }
    focusCurrImage();
}

void ImagesCarousel::unfocusCurrImage() {
    if (m_currentIndex < 0) return;
    if (m_currentIndex >= getLoadedImagesCount()) {
        warn(QString("Invalid index to unfocus: %1").arg(m_currentIndex));
        return;
    }
    auto item = getImageItemAt(m_currentIndex);
    if (item) item->setFocus(false, m_animationEnabled);
}

int ImagesCarousel::_focusingLeftOffset(int index) {
    int spacing      = ui->scrollAreaWidgetContents->layout()->spacing();
    int centerOffset = (m_itemWidth + spacing) * index + m_itemFocusWidth / 2 - spacing;
    return centerOffset - ui->scrollArea->width() / 2;
}

void ImagesCarousel::focusCurrImage() {
    // If no focus, do nothing
    if (m_currentIndex < 0) return;
    if (m_currentIndex >= getLoadedImagesCount()) {
        warn(QString("Invalid index to focus: %1").arg(m_currentIndex));
        return;
    }
    auto item = getImageItemAt(m_currentIndex);
    if (!item) {
        error(QString("Failed to get item at index: %1").arg(m_currentIndex));
        return;
    }
    item->setFocus(true, m_animationEnabled);
    emit imageFocused(item->getFileFullPath(),
                      m_currentIndex,
                      getLoadedImagesCount());
    auto hScrollBar = ui->scrollArea->horizontalScrollBar();
    int leftOffset  = _focusingLeftOffset(m_currentIndex);
    if (leftOffset < 0) {
        leftOffset = 0;
    }

    if (!m_animationEnabled) {
        hScrollBar->setValue(leftOffset);
        return;
    }

    if (m_scrollAnimation) {
        m_scrollAnimation->stop();
        m_scrollAnimation->deleteLater();
        m_scrollAnimation = nullptr;
    }

    m_scrollAnimation = new QPropertyAnimation(hScrollBar, "value");
    m_scrollAnimation->setDuration(s_animationDuration);
    m_scrollAnimation->setStartValue(hScrollBar->value());
    m_scrollAnimation->setEndValue(leftOffset);
    m_scrollAnimation->setEasingCurve(QEasingCurve::OutCubic);

    // Suppress auto focus during animation
    connect(m_scrollAnimation,
            &QPropertyAnimation::finished,
            this,
            [this]() {
                m_suppressAutoFocus = false;
                m_scrollArea->setBlockInput(false);
            });
    m_suppressAutoFocus = true;
    m_scrollArea->setBlockInput(true);
    m_scrollAnimation->start();
}

void ImagesCarousel::_onScrollBarValueChanged(int value) {
    // Stop the animation if it is running
    if (m_scrollAnimation && m_scrollAnimation->state() == QPropertyAnimation::Running) {
        m_scrollAnimation->stop();
        m_scrollAnimation->deleteLater();
        m_scrollAnimation = nullptr;
    }
    int centerOffset = (value + m_itemFocusWidth / 2);
    int itemOffset   = m_itemWidth + ui->scrollAreaWidgetContents->layout()->spacing();
    int index        = centerOffset / itemOffset;

    if (index < 0 || index >= getLoadedImagesCount()) {
        return;  // Out of bounds
    }
    if (index == m_currentIndex) {
        return;  // Already focused
    }
    unfocusCurrImage();
    m_currentIndex = index;
    focusCurrImage();
}

void ImagesCarousel::_onItemClicked(const QString& path) {
    // if (m_suppressAutoFocus) return;
    unfocusCurrImage();
    // Most likely the clicked item is near the current index
    const auto count = getLoadedImagesCount();
    for (int i = m_currentIndex, j = m_currentIndex + 1;
         i >= 0 || j < count;
         --i, ++j) {
        if (i >= 0 && getImageItemAt(i)->getFileFullPath() == path) {
            m_currentIndex = i;
            break;
        }
        if (j < count && getImageItemAt(j)->getFileFullPath() == path) {
            m_currentIndex = j;
            break;
        }
    }
    focusCurrImage();
}

void ImagesCarousel::onStop() {
    QMutexLocker locker(&m_stopSignMutex);
    m_stopSign = true;
}
