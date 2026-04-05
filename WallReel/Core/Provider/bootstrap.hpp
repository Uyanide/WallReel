#ifndef WALLREEL_PROVIDER_BOOTSTRAP_HPP
#define WALLREEL_PROVIDER_BOOTSTRAP_HPP

#include <QQmlEngine>

#include "Cache/manager.hpp"
#include "Config/manager.hpp"
#include "Image/manager.hpp"
#include "Palette/manager.hpp"
#include "Service/manager.hpp"
#include "Utils/misc.hpp"
#include "appoptions.hpp"
#include "logger.hpp"

namespace WallReel::Core::Provider {

class Bootstrap {
    friend class Carousel;

  public:
    Bootstrap(const AppOptions& options) : options(options) {
        configMgr = new Config::Manager(
            Utils::getConfigDir(),
            Utils::getPicturesDir(),
            options.appendDirs,
            options.configPath,
            options.disableActions);

        cacheMgr = new Cache::Manager(
            Utils::getCacheDir(),
            configMgr->getCacheConfig().maxImageEntries);

        if (options.clearCache) {
            cacheMgr->clearCache();
            return;
        }

        imageMgr = new Image::Manager(
            *configMgr,
            *cacheMgr,
            configMgr->getFocusImageSize());

        paletteMgr = new Palette::Manager(
            configMgr->getThemeConfig(),
            *imageMgr);
        qRegisterMetaType<Palette::PaletteItem>("PaletteItem");
        qRegisterMetaType<Palette::ColorItem>("ColorItem");

        serviceMgr = new Service::Manager(
            configMgr->getActionConfig(),
            *imageMgr,
            *paletteMgr,
            options.disableActions);
    }

    void start() {
        cacheMgr->evictOldEntries();
        configMgr->captureState();
        imageMgr->loadAndProcess();
    }

    bool apply(const QString& path) {
        if (options.disableActions) {
            Logger::warn("Bootstrap", "Actions are disabled, cannot apply wallpaper");
            return false;
        }

        QEventLoop loop;
        bool successFlag = false;

        paletteMgr->setSelectedPalette(cacheMgr->getSetting(
            Cache::SettingsType::LastSelectedPalette,
            []() { return Config::CacheConfigItems::defaultSelectedPalette; }));

        QObject::connect(
            configMgr,
            &Config::Manager::stateCaptured,
            &loop,
            [&]() {
                loop.quit();
            },
            Qt::SingleShotConnection);
        configMgr->captureState();
        loop.exec();

        QMetaObject::Connection connection;

        connection = QObject::connect(
            imageMgr,
            &Image::Manager::isLoadingChanged,
            &loop,
            [&]() {
                if (!imageMgr->isLoading()) {
                    QObject::disconnect(connection);
                    QVariant idVar = imageMgr->model()->data(
                        imageMgr->model()->index(0, 0),
                        Image::Model::IdRole);
                    if (idVar.isValid()) {
                        auto id = idVar.toString();
                        paletteMgr->updateColor(id);
                        QObject::connect(
                            serviceMgr,
                            &Service::Manager::selectCompleted,
                            &loop,
                            [&](bool success) {
                                successFlag = success;
                                loop.quit();
                            },
                            Qt::SingleShotConnection);
                        serviceMgr->selectWallpaper(id);
                    } else {
                        Logger::critical("Bootstrap", "No images loaded, cannot apply wallpaper");
                        loop.quit();
                    }
                }
            });

        imageMgr->loadAndProcess({Utils::expandPath(path)});
        loop.exec();
        return successFlag;
    }

    ~Bootstrap() {
        delete serviceMgr;
        delete paletteMgr;
        delete imageMgr;
        delete configMgr;
        delete cacheMgr;
    }

  private:
    const AppOptions& options;
    Cache::Manager* cacheMgr{};
    Config::Manager* configMgr{};
    Image::Manager* imageMgr{};
    Palette::Manager* paletteMgr{};
    Service::Manager* serviceMgr{};
};

}  // namespace WallReel::Core::Provider

#endif  // WALLREEL_PROVIDER_BOOTSTRAP_HPP
