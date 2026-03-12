#include <qobject.h>

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSocketNotifier>

extern "C" {
#include <signal.h>
#include <sys/signalfd.h>
#include <unistd.h>
}

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
    // 4. QSocketNotifier (receives signals for graceful shutdown)
    // 5. QApplication

    // Mask signals for graceful shutdown
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    if (pthread_sigmask(SIG_BLOCK, &mask, nullptr) == -1) {
        // Logger is yet to be initialized, but is still usable with default behavior
        WR_CRITICAL(QString("Failed to block signals: %1").arg(strerror(errno)));
        return 1;
    }

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

        // Create signalfd to receive signals in the Qt event loop
        int sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
        if (sfd == -1) {
            WR_CRITICAL(QString("Failed to create signalfd: %1").arg(strerror(errno)));
            return 1;
        }
        QSocketNotifier notifier(sfd, QSocketNotifier::Read, &a);

        QObject::connect(
            &notifier,
            &QSocketNotifier::activated,
            &a,
            [sfd, &a]() {
                struct signalfd_siginfo fdsi;
                ssize_t s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
                if (s == sizeof(struct signalfd_siginfo)) {
                    WR_DEBUG(QString("Received signal: %1").arg(fdsi.ssi_signo));
                    a.quit();
                }
            });

        AppOptions options;
        options.parseArgs(a);

        if (options.doReturn) {
            return options.errorText.isEmpty() ? 0 : 1;
        }

        Provider::Bootstrap bootstrap(options);

        if (options.clearCache) {
            return 0;
        }

        if (!options.applyPath.isEmpty()) {
            return bootstrap.apply(options.applyPath) ? 0 : 1;
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
