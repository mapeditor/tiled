#include <QApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString qmlDir = QApplication::applicationDirPath();
#ifdef Q_OS_WIN
    qmlDir += QStringLiteral("/qml");
#else
    qmlDir += QStringLiteral("/../qml");
#endif

    QQmlApplicationEngine engine;
    engine.addImportPath(qmlDir);

    QString mainQml(qmlDir + QStringLiteral("/tiledquick/main.qml"));
    engine.load(QUrl::fromLocalFile(mainQml));

    return app.exec();
}
