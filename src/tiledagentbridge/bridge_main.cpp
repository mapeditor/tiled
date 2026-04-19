#include "bridgeengine.h"
#include "jsonrpcserver.h"

#include <QGuiApplication>

int main(int argc, char *argv[])
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("tiledagentbridge"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));

    tiledagent::BridgeEngine engine;
    tiledagent::JsonRpcServer server(engine);
    return server.run();
}
