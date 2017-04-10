#include <QApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationDomain("mapeditor.org");
    QCoreApplication::setApplicationName("TiledQuick");

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
