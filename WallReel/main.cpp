#include <qobject.h>

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "Core/Config/manager.hpp"
#include "Core/Image/model.hpp"
#include "Core/Palette/data.hpp"
#include "Core/Palette/manager.hpp"
#include "Core/Service/manager.hpp"
#include "Core/Utils/misc.hpp"
#include "Core/appoptions.hpp"
#include "Core/logger.hpp"
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

    auto config = new Config::Manager(
        Utils::getConfigDir(),
        s_options.appendDirs,
        s_options.configPath,
        &engine);
    qmlRegisterSingletonInstance(
        COREMODULE_URI,
        MODULE_VERSION_MAJOR,
        MODULE_VERSION_MINOR,
        "Config",
        config);

    auto imageModel = new Image::Model(
        config->getSortConfig(),
        Utils::getCacheDir(),
        config->getFocusImageSize(),
        config);
    qmlRegisterSingletonInstance(
        COREMODULE_URI,
        MODULE_VERSION_MAJOR,
        MODULE_VERSION_MINOR,
        "ImageModel",
        imageModel);

    auto paletteMgr = new Palette::Manager(
        config->getThemeConfig(),
        *imageModel,
        imageModel);
    engine.rootContext()->setContextProperty("PaletteManager", paletteMgr);
    qRegisterMetaType<Palette::PaletteItem>("PaletteItem");
    qRegisterMetaType<Palette::ColorItem>("ColorItem");

    auto Service = new Service::Manager(
        config->getActionConfig(),
        *imageModel,
        *paletteMgr,
        paletteMgr);
    qmlRegisterSingletonInstance(
        COREMODULE_URI,
        MODULE_VERSION_MAJOR,
        MODULE_VERSION_MINOR,
        "ServiceManager",
        Service);
    if (config->getActionConfig().quitOnSelected) {
        QObject::connect(
            Service,
            &Service::Manager::selectCompleted,
            &a,
            []() { QCoreApplication::quit(); });
    }
    QObject::connect(
        Service,
        &Service::Manager::cancelCompleted,
        &a,
        []() { QCoreApplication::quit(); });

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &a,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule(UIMODULE_URI, "Main");

    config->captureState();
    imageModel->loadAndProcess(config->getWallpapers());

    return a.exec();
}
