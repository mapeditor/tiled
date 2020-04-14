#include <QApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationDomain("mapeditor.org");
    QCoreApplication::setApplicationName("TiledQuick");

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    // We don't need the scaling factor to be rounded
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication app(argc, argv);

    QString qmlDir = QApplication::applicationDirPath();
#ifdef Q_OS_WIN
    qmlDir += QStringLiteral("/qml");
#else
    qmlDir += QStringLiteral("/../qml");
#endif

    QQmlApplicationEngine engine;
    engine.addImportPath(qmlDir);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
