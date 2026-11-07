#include "editableasset.h"
#include "map.h"
#include "mapdocument.h"
#include "mcpserver.h"
#include "projectmanager.h"
#include "scriptmanager.h"

#include <QJSEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QUndoStack>
#include <QtTest/QtTest>

#include <memory>

using namespace Tiled;

class test_McpServer : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE bool nestedScriptRejected()
    {
        return execute("return 1;").value("isError").toBool();
    }

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    void discovery();
    void accessControl();
    void invalidRequests_data();
    void invalidRequests();
    void scripts_data();
    void scripts();
    void notificationsDoNotExecute();
    void editsUseDocumentUndo();
    void restartRevokesToken();
    void reentrantScript();

private:
    QNetworkReply *post(
        const QByteArray &body,
        const QByteArray &token,
        const QByteArray &header = {},
        const QByteArray &value = {});
    QJsonObject call(const QString &method, const QJsonObject &params = {});
    QJsonObject execute(const QString &code);

    std::unique_ptr<ProjectManager> mProjectManager;
    McpServer mServer;
    QNetworkAccessManager mNetwork;
};

void test_McpServer::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    mProjectManager = std::make_unique<ProjectManager>();
    ScriptManager::instance().ensureInitialized();
}

void test_McpServer::cleanupTestCase()
{
    ScriptManager::deleteInstance();
    mProjectManager.reset();
}

void test_McpServer::init()
{
    QVERIFY(mServer.start());
}

void test_McpServer::cleanup()
{
    mServer.stop();
}

QNetworkReply *test_McpServer::post(
    const QByteArray &body,
    const QByteArray &token,
    const QByteArray &header,
    const QByteArray &value)
{
    QNetworkRequest request{QUrl(mServer.url())};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json, text/event-stream");
    request.setRawHeader("Authorization", "Bearer " + token);
    if (!header.isEmpty())
        request.setRawHeader(header, value);
    auto reply = mNetwork.post(request, body);
    reply->setParent(this);
    QSignalSpy finished(reply, &QNetworkReply::finished);
    if (!finished.wait(5000))
        QTest::qFail("HTTP request timed out", __FILE__, __LINE__);
    reply->deleteLater();
    return reply;
}

QJsonObject test_McpServer::call(const QString &method, const QJsonObject &params)
{
    const QJsonObject request{{"jsonrpc", "2.0"}, {"id", 1}, {"method", method}, {"params", params}};
    auto reply = post(QJsonDocument(request).toJson(), mServer.token());
    return QJsonDocument::fromJson(reply->readAll()).object();
}

QJsonObject test_McpServer::execute(const QString &code)
{
    return call("tools/call", {{"name", "execute_script"}, {"arguments", QJsonObject{{"code", code}}}})
        .value("result")
        .toObject();
}

void test_McpServer::discovery()
{
    const auto initialized = call(
                                 "initialize",
                                 {{"protocolVersion", "2025-11-25"},
                                  {"clientInfo", QJsonObject{{"name", "test"}, {"version", "1"}}},
                                  {"capabilities", QJsonObject{}}})
                                 .value("result")
                                 .toObject();
    QCOMPARE(initialized.value("protocolVersion").toString(), QString("2025-11-25"));
    QVERIFY(initialized.value("capabilities").toObject().contains("tools"));
    const auto tools = call("tools/list").value("result").toObject().value("tools").toArray();
    QCOMPARE(tools.size(), 1);
    QCOMPARE(tools.first().toObject().value("name").toString(), QString("execute_script"));
    const auto resources
        = call("resources/list").value("result").toObject().value("resources").toArray();
    QCOMPARE(resources.size(), 1);
    const auto api = call("resources/read", {{"uri", "tiled://scripting-api"}})
                         .value("result")
                         .toObject()
                         .value("contents")
                         .toArray();
    QCOMPARE(api.size(), 1);
    QVERIFY(api.first().toObject().value("text").toString().contains("declare namespace tiled"));
}

void test_McpServer::accessControl()
{
    const QByteArray ping = R"({"jsonrpc":"2.0","id":1,"method":"ping"})";
    QCOMPARE(post(ping, {})->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 401);
    QCOMPARE(
        post(ping, mServer.token(), "Origin", "http://localhost")
            ->attribute(QNetworkRequest::HttpStatusCodeAttribute)
            .toInt(),
        403);
    QCOMPARE(
        post(ping, mServer.token(), "Host", "example.com")
            ->attribute(QNetworkRequest::HttpStatusCodeAttribute)
            .toInt(),
        403);
    QCOMPARE(
        post(ping, mServer.token(), "Content-Type", "text/plain")
            ->attribute(QNetworkRequest::HttpStatusCodeAttribute)
            .toInt(),
        415);
    QCOMPARE(
        post(ping, mServer.token(), "Accept", "text/html")
            ->attribute(QNetworkRequest::HttpStatusCodeAttribute)
            .toInt(),
        406);
    QCOMPARE(
        post(ping, mServer.token(), "MCP-Protocol-Version", "invalid")
            ->attribute(QNetworkRequest::HttpStatusCodeAttribute)
            .toInt(),
        400);
    QCOMPARE(
        post(QByteArray(1024 * 1024 + 1, 'x'), mServer.token())
            ->attribute(QNetworkRequest::HttpStatusCodeAttribute)
            .toInt(),
        413);
}

void test_McpServer::invalidRequests_data()
{
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<int>("code");
    QTest::newRow("parse") << QByteArray("{") << -32700;
    QTest::newRow("batch") << QByteArray("[]") << -32600;
    QTest::newRow("version") << QByteArray(R"({"jsonrpc":"1.0","id":1,"method":"ping"})") << -32600;
    QTest::newRow("id") << QByteArray(R"({"jsonrpc":"2.0","id":null,"method":"ping"})") << -32600;
    QTest::newRow("params") << QByteArray(R"({"jsonrpc":"2.0","id":1,"method":"ping","params":[]})")
                            << -32600;
    QTest::newRow("method") << QByteArray(R"({"jsonrpc":"2.0","id":1,"method":"missing"})")
                            << -32601;
    QTest::newRow("initialize") << QByteArray(R"({"jsonrpc":"2.0","id":1,"method":"initialize"})")
                                << -32602;
    QTest::newRow("tool") << QByteArray(
        R"({"jsonrpc":"2.0","id":1,"method":"tools/call","params":{"name":"missing"}})")
                          << -32602;
    QTest::newRow("arguments") << QByteArray(
        R"({"jsonrpc":"2.0","id":1,"method":"tools/call","params":{"name":"execute_script","arguments":{"code":1}}})")
                               << -32602;
    QTest::newRow("resource") << QByteArray(
        R"({"jsonrpc":"2.0","id":1,"method":"resources/read","params":{"uri":"missing"}})")
                              << -32002;
}

void test_McpServer::invalidRequests()
{
    QFETCH(QByteArray, body);
    QFETCH(int, code);
    const auto response = QJsonDocument::fromJson(post(body, mServer.token())->readAll()).object();
    QCOMPARE(response.value("jsonrpc").toString(), QString("2.0"));
    QCOMPARE(response.value("error").toObject().value("code").toInt(), code);
}

void test_McpServer::scripts_data()
{
    QTest::addColumn<QByteArray>("code");
    QTest::addColumn<bool>("isError");
    QTest::addColumn<QByteArray>("text");
    QTest::newRow("json") << QByteArray("return {answer: 42};") << false
                          << QByteArray("{\"answer\":42}");
    QTest::newRow("undefined") << QByteArray("var local = 1;") << false << QByteArray("null");
    QTest::newRow("syntax") << QByteArray("return (;") << true << QByteArray("SyntaxError");
    QTest::newRow("error") << QByteArray("throw new Error('failure');") << true
                           << QByteArray("failure");
    QTest::newRow("primitive-error")
        << QByteArray("throw 'failure';") << true << QByteArray("failure");
    QTest::newRow("cyclic") << QByteArray("var x = {}; x.x = x; return x;") << true
                            << QByteArray("TypeError");
    QTest::newRow("scope") << QByteArray("return typeof local;") << false
                           << QByteArray("\"undefined\"");
}

void test_McpServer::scripts()
{
    QFETCH(QByteArray, code);
    QFETCH(bool, isError);
    QFETCH(QByteArray, text);
    const auto response = execute(QString::fromUtf8(code));
    QCOMPARE(response.value("isError").toBool(), isError);
    QVERIFY(response.value("content").toArray().first().toObject().value("text").toString().contains(
        QString::fromUtf8(text)));
}

void test_McpServer::notificationsDoNotExecute()
{
    auto reply = post(
        R"({"jsonrpc":"2.0","method":"tools/call","params":{"name":"execute_script","arguments":{"code":"globalThis.notificationRan = true;"}}})",
        mServer.token());
    QCOMPARE(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), 202);
    QVERIFY(reply->readAll().isEmpty());
    QVERIFY(
        ScriptManager::instance().engine()->globalObject().property("notificationRan").isUndefined());
}

void test_McpServer::editsUseDocumentUndo()
{
    MapDocument document(std::make_unique<Map>(Map::Orthogonal, 4, 4, 16, 16));
    auto engine = ScriptManager::instance().engine();
    QJSEngine::setObjectOwnership(document.editable(), QJSEngine::CppOwnership);
    engine->globalObject().setProperty("testMap", engine->newQObject(document.editable()));
    auto response = execute("testMap.macro('Edit map', function() { testMap.setProperty('answer', "
                            "42); testMap.setProperty('other', 7); }); return testMap.modified;");
    QVERIFY(!response.value("isError").toBool());
    QCOMPARE(document.map()->property("answer").toInt(), 42);
    QCOMPARE(document.undoStack()->count(), 1);
    QVERIFY(document.isModified());
    document.undoStack()->undo();
    QVERIFY(!document.map()->hasProperty("answer"));
    QVERIFY(!document.map()->hasProperty("other"));
    QVERIFY(!document.isModified());
    engine->globalObject().deleteProperty("testMap");
}

void test_McpServer::restartRevokesToken()
{
    const auto oldToken = mServer.token();
    mServer.stop();
    QVERIFY(mServer.token().isEmpty());
    QVERIFY(mServer.start());
    QVERIFY(mServer.token() != oldToken);
    QCOMPARE(
        post(R"({"jsonrpc":"2.0","id":1,"method":"ping"})", oldToken)
            ->attribute(QNetworkRequest::HttpStatusCodeAttribute)
            .toInt(),
        401);
    QVERIFY(call("ping").contains("result"));
}

void test_McpServer::reentrantScript()
{
    auto engine = ScriptManager::instance().engine();
    QJSEngine::setObjectOwnership(this, QJSEngine::CppOwnership);
    engine->globalObject().setProperty("testHost", engine->newQObject(this));
    const auto response = execute("return testHost.nestedScriptRejected();");
    QCOMPARE(
        response.value("content").toArray().first().toObject().value("text").toString(),
        QString("true"));
    QVERIFY(!execute("return 2;").value("isError").toBool());
    engine->globalObject().deleteProperty("testHost");
}

QTEST_MAIN(test_McpServer)

#include "test_mcpserver.moc"
