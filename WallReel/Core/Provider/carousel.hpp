#ifndef WALLREEL_PROVIDER_CAROUSEL_HPP
#define WALLREEL_PROVIDER_CAROUSEL_HPP

#include <qapplication.h>

#include <QApplication>

#include "Cache/manager.hpp"
#include "Cache/types.hpp"
#include "Config/data.hpp"
#include "Config/manager.hpp"
#include "Image/manager.hpp"
#include "Palette/data.hpp"
#include "Palette/manager.hpp"
#include "Service/manager.hpp"
#include "bootstrap.hpp"

namespace WallReel::Core::Provider {

class Carousel : public QObject {
    Q_OBJECT

    // Image::Manager

  public:
    Q_PROPERTY(Image::ProxyModel* imageModel READ imageModel CONSTANT)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(int processedCount READ processedCount NOTIFY processedCountChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)
    Q_PROPERTY(QString sortType READ sortType NOTIFY sortTypeChanged)
    Q_PROPERTY(bool sortDescending READ sortDescending NOTIFY sortDescendingChanged)
    Q_PROPERTY(QString searchText READ searchText NOTIFY searchTextChanged)

    Image::ProxyModel* imageModel() const { return m_imageMgr->model(); }

    bool isLoading() const { return m_imageMgr->isLoading(); }

    int processedCount() const { return m_imageMgr->processedCount(); }

    QString sortType() const { return Config::sortTypeToString(m_imageMgr->sortType()); }

    bool sortDescending() const { return m_imageMgr->sortDescending(); }

    QString searchText() const { return m_imageMgr->searchText(); }

    int totalCount() const { return m_imageMgr->totalCount(); }

    Q_INVOKABLE void stopLoading() { m_imageMgr->stop(); }

    Q_INVOKABLE void setSortType(const QString& sortTypeStr) {
        auto newSortType = Config::stringToSortType(sortTypeStr);
        m_imageMgr->setSortType(newSortType);
        emit sortTypeChanged();
    }

    Q_INVOKABLE void setSortType(Config::SortType sortType) {
        m_imageMgr->setSortType(sortType);
        emit sortTypeChanged();
    }

    Q_INVOKABLE void setSortDescending(bool descending) {
        m_imageMgr->setSortDescending(descending);
        emit sortDescendingChanged();
    }

    Q_INVOKABLE void setSearchText(const QString& text) {
        m_imageMgr->setSearchText(text);
        emit searchTextChanged();
    }

  signals:
    void isLoadingChanged();
    void processedCountChanged();
    void totalCountChanged();
    void sortTypeChanged();
    void sortDescendingChanged();
    void searchTextChanged();

    // Config::Manager

  public:
    Q_PROPERTY(int imageWidth READ imageWidth CONSTANT)
    Q_PROPERTY(int imageHeight READ imageHeight CONSTANT)
    Q_PROPERTY(double imageFocusScale READ imageFocusScale CONSTANT)
    Q_PROPERTY(int windowWidth READ windowWidth CONSTANT)
    Q_PROPERTY(int windowHeight READ windowHeight CONSTANT)
    Q_PROPERTY(QStringList availableSortTypes READ availableSortTypes CONSTANT)

    int imageWidth() const { return m_configMgr->getStyleConfig().imageWidth; }

    int imageHeight() const { return m_configMgr->getStyleConfig().imageHeight; }

    int windowWidth() const { return m_configMgr->getStyleConfig().windowWidth; }

    int windowHeight() const { return m_configMgr->getStyleConfig().windowHeight; }

    double imageFocusScale() const { return m_configMgr->getStyleConfig().imageFocusScale; }

    QStringList availableSortTypes() const { return Config::s_availableSortTypes; }

    // Palette::Manager

  public:
    Q_PROPERTY(QList<Palette::PaletteItem> availablePalettes READ availablePalettes CONSTANT)
    Q_PROPERTY(QVariant selectedPalette READ selectedPalette NOTIFY selectedPaletteChanged)
    Q_PROPERTY(QVariant selectedColor READ selectedColor NOTIFY selectedColorChanged)
    Q_PROPERTY(QColor color READ color NOTIFY colorChanged)
    Q_PROPERTY(QString colorName READ colorName NOTIFY colorNameChanged)

    QList<Palette::PaletteItem> availablePalettes() const { return m_paletteMgr->availablePalettes(); }

    QVariant selectedPalette() const { return m_paletteMgr->selectedPalette(); }

    QVariant selectedColor() const { return m_paletteMgr->selectedColor(); }

    QColor color() const { return m_paletteMgr->color(); }

    QString colorName() const { return m_paletteMgr->colorName(); }

    Q_INVOKABLE void requestSelectPalette(const QVariant& palette) { m_paletteMgr->setSelectedPalette(palette); }

    Q_INVOKABLE void requestSelectColor(const QVariant& color) { m_paletteMgr->setSelectedColor(color); }

  signals:
    void selectedPaletteChanged();
    void selectedColorChanged();
    void colorChanged();
    void colorNameChanged();

    // Service::Manager

  public:
    Q_PROPERTY(bool isProcessing READ isProcessing NOTIFY isProcessingChanged)

    bool isProcessing() const { return m_serviceMgr->isProcessing(); }

    Q_INVOKABLE void confirm() {
        if (!m_currentImageId.isEmpty()) {
            m_serviceMgr->selectWallpaper(m_currentImageId);
        }
    }

    Q_INVOKABLE void cancel() { m_serviceMgr->cancel(); }

    Q_INVOKABLE void restore() { m_serviceMgr->restore(); }

    Q_INVOKABLE bool hasSelected() const { return m_serviceMgr->hasSelected(); }

  signals:
    void isProcessingChanged();
    void selectCompleted();
    void previewCompleted();
    void restoreCompleted();
    void cancelCompleted();

    // Other states

  public:
    Q_PROPERTY(QString currentImageId READ currentImageId NOTIFY currentImageIdChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)

    QString currentImageId() const { return m_currentImageId; }

    int currentIndex() const { return m_currentIndex; }

    Q_INVOKABLE void setCurrentImageId(const QString& imageId) {
        if (m_currentImageId != imageId) {
            m_currentImageId = imageId;
            emit currentImageIdChanged();
        }
    }

    Q_INVOKABLE void setCurrentIndex(int index) {
        if (m_currentIndex != index) {
            m_currentIndex = index;
            emit currentIndexChanged();
        }
    }

  signals:
    void currentImageIdChanged();
    void currentIndexChanged();

  private:
    QString m_currentImageId;
    int m_currentIndex = -1;

    // instances

  public:
    Carousel(QApplication* app,
             Bootstrap& bootstrap,
             QObject* parent = nullptr)
        : QObject(parent),
          m_cacheMgr(bootstrap.cacheMgr),
          m_configMgr(bootstrap.configMgr),
          m_imageMgr(bootstrap.imageMgr),
          m_paletteMgr(bootstrap.paletteMgr),
          m_serviceMgr(bootstrap.ServiceMgr) {
        // Simply forward signals
        connect(m_imageMgr, &Image::Manager::isLoadingChanged, this, &Carousel::isLoadingChanged);
        connect(m_imageMgr, &Image::Manager::processedCountChanged, this, &Carousel::processedCountChanged);
        connect(m_imageMgr, &Image::Manager::totalCountChanged, this, &Carousel::totalCountChanged);
        connect(m_paletteMgr, &Palette::Manager::selectedPaletteChanged, this, &Carousel::selectedPaletteChanged);
        connect(m_paletteMgr, &Palette::Manager::selectedColorChanged, this, &Carousel::selectedColorChanged);
        connect(m_paletteMgr, &Palette::Manager::colorChanged, this, &Carousel::colorChanged);
        connect(m_paletteMgr, &Palette::Manager::colorNameChanged, this, &Carousel::colorNameChanged);
        connect(m_serviceMgr, &Service::Manager::isProcessingChanged, this, &Carousel::isProcessingChanged);
        connect(m_serviceMgr, &Service::Manager::selectCompleted, this, &Carousel::selectCompleted);
        connect(m_serviceMgr, &Service::Manager::previewCompleted, this, &Carousel::previewCompleted);
        connect(m_serviceMgr, &Service::Manager::restoreCompleted, this, &Carousel::restoreCompleted);
        connect(m_serviceMgr, &Service::Manager::cancelCompleted, this, &Carousel::cancelCompleted);

        // "Preview" is costly, but is (usually) protected by a debounce timer, so it seems fine
        // to call it multiple times in a short period, and it simplifies the code a lot :)

        // Preview on imageid change
        connect(this, &Carousel::currentImageIdChanged, this, [this]() {
            if (!m_currentImageId.isEmpty()) {
                m_serviceMgr->previewWallpaper(m_currentImageId);
            }
        });
        // Update displayed color when imageid changes
        connect(this, &Carousel::currentImageIdChanged, this, [this]() {
            if (!m_currentImageId.isEmpty()) {
                m_paletteMgr->updateColor(m_currentImageId);
            }
        });
        // Update displayed color when selected color changes
        connect(this, &Carousel::selectedColorChanged, this, [this]() {
            if (!m_currentImageId.isEmpty()) {
                m_paletteMgr->updateColor(m_currentImageId);
            }
        });
        // Preview on selected palette change
        connect(this, &Carousel::selectedPaletteChanged, this, [this]() {
            if (!m_currentImageId.isEmpty()) {
                m_serviceMgr->previewWallpaper(m_currentImageId);
            }
        });
        // Preview on displayed color name change
        connect(this, &Carousel::colorNameChanged, this, [this]() {
            if (!m_currentImageId.isEmpty()) {
                m_serviceMgr->previewWallpaper(m_currentImageId);
            }
        });
        // Preview on displayed color hex change
        connect(this, &Carousel::colorChanged, this, [this]() {
            if (!m_currentImageId.isEmpty()) {
                m_serviceMgr->previewWallpaper(m_currentImageId);
            }
        });

        // Quit on selected
        if (m_configMgr->getActionConfig().quitOnSelected) {
            QObject::connect(
                this,
                &Provider::Carousel::selectCompleted,
                app,
                &QApplication::quit,
                Qt::QueuedConnection);
        }
        // Quit on cancel
        QObject::connect(
            this,
            &Provider::Carousel::cancelCompleted,
            app,
            &QApplication::quit,
            Qt::QueuedConnection);
        // Restore on quit
        if (m_configMgr->getActionConfig().restoreOnClose) {
            QObject::connect(
                app,
                &QApplication::aboutToQuit,
                m_serviceMgr,
                &Service::Manager::restoreOnQuit);
        }

        // Restore last state if configured
        // and store state on change if configured
        // Note: connect after restoring state to avoid storing the restored state again
        if (m_configMgr->getCacheConfig().saveSortMethod) {
            setSortType(m_cacheMgr->getSetting(
                Cache::SettingsType::LastSortType,
                []() { return Config::CacheConfigItems::defaultSortType; }));
            setSortDescending(m_cacheMgr->getSetting(
                                  Cache::SettingsType::LastSortDescending,
                                  []() { return Config::CacheConfigItems::defaultSortDescending; }) == "true");
            connect(this, &Carousel::sortTypeChanged, this, [this]() {
                m_cacheMgr->storeSetting(
                    Cache::SettingsType::LastSortType,
                    Config::sortTypeToString(m_imageMgr->sortType()));
            });
            connect(this, &Carousel::sortDescendingChanged, this, [this]() {
                m_cacheMgr->storeSetting(
                    Cache::SettingsType::LastSortDescending,
                    m_imageMgr->sortDescending() ? "true" : "false");
            });
        }
        if (m_configMgr->getCacheConfig().savePalette) {
            requestSelectPalette(m_cacheMgr->getSetting(
                Cache::SettingsType::LastSelectedPalette,
                []() { return Config::CacheConfigItems::defaultSelectedPalette; }));
            connect(this, &Carousel::selectedPaletteChanged, this, [this]() {
                m_cacheMgr->storeSetting(
                    Cache::SettingsType::LastSelectedPalette,
                    m_paletteMgr->getSelectedPaletteName());
            });
        }
    }

  private:
    Cache::Manager* m_cacheMgr;
    Config::Manager* m_configMgr;
    Image::Manager* m_imageMgr;
    Palette::Manager* m_paletteMgr;
    Service::Manager* m_serviceMgr;
};

}  // namespace WallReel::Core::Provider

#endif  // WALLREEL_PROVIDER_CAROUSEL_HPP
