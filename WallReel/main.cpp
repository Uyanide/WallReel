#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "Core/Config/manager.hpp"
#include "Core/Image/model.hpp"
#include "Core/Image/provider.hpp"
#include "Core/Palette/data.hpp"
#include "Core/Palette/manager.hpp"
#include "Core/Utils/misc.hpp"
#include "Core/appoptions.hpp"
#include "Core/logger.hpp"
#include "Core/wallpaperservice.hpp"
#include "version.h"

using namespace WallReel::Core;

int main(int argc, char* argv[]) {
    AppOptions s_options;

    QApplication a(argc, argv);
    a.setApplicationName(APP_NAME);
    a.setApplicationVersion(APP_VERSION);

    Logger::init();
    s_options.parseArgs(a);

    if (s_options.doReturn) {
        return s_options.errorText.isEmpty() ? 0 : 1;
    }

    QQmlApplicationEngine engine;

    auto* imageProvider = new Image::Provider();
    engine.addImageProvider(QLatin1String("processed"), imageProvider);

    auto config = new Config::Manager(
        Utils::getConfigDir(),
        s_options.appendDirs,
        s_options.configPath,
        imageProvider);
    qmlRegisterSingletonInstance(
        COREMODULE_URI,
        MODULE_VERSION_MAJOR,
        MODULE_VERSION_MINOR,
        "Config",
        config);

    auto paletteMgr = new Palette::Manager(
        config->getPaletteConfig(),
        &a);
    engine.rootContext()->setContextProperty("PaletteManager", paletteMgr);
    qRegisterMetaType<Palette::PaletteItem>("PaletteItem");
    qRegisterMetaType<Palette::ColorItem>("ColorItem");

    auto imageModel = new Image::Model(
        *imageProvider,
        config->getSortConfig(),
        config->getFocusImageSize(),
        config);
    qmlRegisterSingletonInstance(
        COREMODULE_URI,
        MODULE_VERSION_MAJOR,
        MODULE_VERSION_MINOR,
        "ImageModel",
        imageModel);

    auto wallpaperService = new WallpaperService(
        config->getActionConfig(),
        config);
    QObject::connect(
        imageModel,
        &Image::Model::imageSelected,
        wallpaperService,
        &WallpaperService::select);
    QObject::connect(
        imageModel,
        &Image::Model::imagePreviewed,
        wallpaperService,
        &WallpaperService::preview);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &a,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule(UIMODULE_URI, "Main");

    imageModel->loadAndProcess(config->getWallpapers());

    return a.exec();
}
