// SPDX-License-Identifier: BSD-3-Clause

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>
#include <QUrl>

#include <MauiKit4/Core/mauiapp.h>

#include "controllers/valenzbridge.h"

int main(int argc, char *argv[])
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    QSurfaceFormat::setDefaultFormat(format);

    QGuiApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("Maui"));

    // Ensure MauiKit core is initialized so Maui QML resources are available.
    MauiApp::instance();

    QQmlApplicationEngine engine;
    ValenzBridge valenzBridge;
    engine.rootContext()->setContextProperty(QStringLiteral("valenzBridge"), &valenzBridge);

    const QUrl url(QStringLiteral("qrc:/app/valenz/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                     [url](QObject *obj, const QUrl &objUrl)
    {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
