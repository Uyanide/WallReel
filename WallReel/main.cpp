#include <qapplication.h>
#include <qobject.h>

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "Core/Provider/bootstrap.hpp"
#include "Core/Provider/carousel.hpp"
#include "Core/appoptions.hpp"
#include "Core/logger.hpp"
#include "version.h"

using namespace WallReel::Core;

WALLREEL_DECLARE_SENDER("Main")

int main(int argc, char* argv[]) {
    // Destruction order after QApplication quits its event loop:
    // 1. QQmlApplicationEngine (with all QML objects)
    // 2. provider (manages states and connections)
    // 3. bootstrap (manages lifecycle of all managers)
    // 4. QApplication

    QApplication a(argc, argv);
    a.setApplicationName(APP_NAME);
    a.setApplicationVersion(APP_VERSION);
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    using namespace Qt::StringLiterals;
    a.setWindowIcon(QIcon(u":/%1.svg"_s.arg(APP_NAME)));
#else
    a.setWindowIcon(QIcon(u":/%1.svg"_qs.arg(APP_NAME)));
#endif

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

        {
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
                using namespace Qt::StringLiterals;
                engine.loadFromModule(UIMODULE_URI, u"Main"_s);
#else
                engine.addImportPath(u"qrc:/"_qs);
                engine.load(QUrl(u"qrc:/WallReel/UI/Main.qml"_qs));
#endif

                bootstrap.start();

                return a.exec();
            }
        }
    }
}
