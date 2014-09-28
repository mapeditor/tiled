#include <QApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
#ifdef Q_OS_WIN
    engine.addImportPath(QApplication::applicationDirPath() + "/qml");
#else
    engine.addImportPath(QApplication::applicationDirPath() + "/../qml");
#endif

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    return app.exec();
}
