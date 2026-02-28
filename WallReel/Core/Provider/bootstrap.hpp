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

namespace WallReel::Core::Provider {

class Bootstrap {
    friend class Carousel;

  public:
    Bootstrap(const AppOptions& options) {
        cacheMgr = new Cache::Manager(Utils::getCacheDir());

        if (options.clearCache) {
            cacheMgr->clearCache();
            return;
        }
        configMgr = new Config::Manager(
            Utils::getConfigDir(),
            options.appendDirs,
            options.configPath);

        imageMgr = new Image::Manager(
            *cacheMgr,
            configMgr->getFocusImageSize());

        paletteMgr = new Palette::Manager(
            configMgr->getThemeConfig(),
            *imageMgr);
        qRegisterMetaType<Palette::PaletteItem>("PaletteItem");
        qRegisterMetaType<Palette::ColorItem>("ColorItem");

        ServiceMgr = new Service::Manager(
            configMgr->getActionConfig(),
            *imageMgr,
            *paletteMgr);
    }

    void start() {
        configMgr->captureState();
        imageMgr->loadAndProcess(configMgr->getWallpapers());
    }

    ~Bootstrap() {
        delete ServiceMgr;
        delete paletteMgr;
        delete imageMgr;
        delete configMgr;
        delete cacheMgr;
    }

  private:
    Cache::Manager* cacheMgr;
    Config::Manager* configMgr;
    Image::Manager* imageMgr;
    Palette::Manager* paletteMgr;
    Service::Manager* ServiceMgr;
};

}  // namespace WallReel::Core::Provider

#endif  // WALLREEL_PROVIDER_BOOTSTRAP_HPP
