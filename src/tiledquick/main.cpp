#include <QApplication>
#include <QQmlApplicationEngine>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationDomain("mapeditor.org");
    QCoreApplication::setApplicationName("TiledQuick");

    // Not rounding causes pixel movement during panning and window resizing
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);

    QApplication app(argc, argv);

    QString qmlDir = QApplication::applicationDirPath();
#ifdef Q_OS_WIN
    qmlDir += QStringLiteral("/qml");
#else
    qmlDir += QStringLiteral("/../qml");
#endif

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    // format.setSamples(8);       // Increase quality of lines and edges when UI is scaled down
    format.setSwapInterval(0);  // Disable vsync
    QSurfaceFormat::setDefaultFormat(format);

    QQmlApplicationEngine engine;
    engine.addImportPath(qmlDir);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return QApplication::exec();
}
