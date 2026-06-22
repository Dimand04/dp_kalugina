#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "networkmanager.h"
#include "barcodescanner.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    app.setOrganizationName("RailwayTech");
    app.setOrganizationDomain("tech.rzd");
    app.setApplicationName("TSD_Inventory");

    qmlRegisterType<BarcodeScanner>("AppComponents", 1, 0, "BarcodeScanner");

    QQmlApplicationEngine engine;

    NetworkManager networkManager;
    engine.rootContext()->setContextProperty("netManager", &networkManager);

    const QUrl url(u"qrc:/dp_kalugina_mobile/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
                     Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
