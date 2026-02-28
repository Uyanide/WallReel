#include <qapplication.h>
#include <qobject.h>

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQml/QQmlExtensionPlugin>

Q_IMPORT_QML_PLUGIN(WallReel_CorePlugin)
Q_IMPORT_QML_PLUGIN(WallReel_UIPlugin)

#include "Core/Provider/bootstrap.hpp"
#include "Core/Provider/carousel.hpp"
#include "Core/appoptions.hpp"
#include "Core/logger.hpp"
#include "version.h"

using namespace WallReel::Core;

WALLREEL_DECLARE_SENDER("Main")

int main(int argc, char* argv[]) {

    QApplication a(argc, argv);
    a.setApplicationName(APP_NAME);
    a.setApplicationVersion(APP_VERSION);
    a.setWindowIcon(QIcon(QString(":/%1.svg").arg(APP_NAME)));

    {
        Logger::init();

        AppOptions options;
        options.parseArgs(a);

        if (options.doReturn) {
            return options.errorText.isEmpty() ? 0 : 1;
        }

        Provider::Bootstrap bootstrap(options);

        if (options.clearCache) {
            return 0;
        }

        Provider::Carousel provider(&a, bootstrap);
        qmlRegisterSingletonInstance(
            COREMODULE_URI,
            MODULE_VERSION_MAJOR,
            MODULE_VERSION_MINOR,
            "CarouselProvider",
            &provider);
        {
            QQmlApplicationEngine engine;

            QObject::connect(
                &engine,
                &QQmlApplicationEngine::objectCreationFailed,
                &a,
                []() { QCoreApplication::exit(-1); },
                Qt::QueuedConnection);
            engine.loadFromModule(UIMODULE_URI, "Main");

            provider.start();

            return a.exec();
        }
    }
}
