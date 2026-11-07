/*
 * This file is part of Tiled, licensed under the GNU GPL, version 2 or later.
 */

#include "mcpserver.h"

#include "scriptmanager.h"

#include <QCheckBox>
#include <QClipboard>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QGuiApplication>
#include <QHttpServerRequest>
#include <QJSEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScopedValueRollback>
#include <QUuid>
#include <QVBoxLayout>

namespace Tiled {

namespace {

const QString protocolVersion = QStringLiteral("2025-11-25");
const QString apiUri = QStringLiteral("tiled://scripting-api");

bool supportedVersion(const QString &version)
{
    return version == protocolVersion || version == QLatin1String("2025-06-18")
            || version == QLatin1String("2025-03-26");
}

QJsonObject error(int code, const QString &message)
{
    return {{QStringLiteral("error"), QJsonObject {
        {QStringLiteral("code"), code}, {QStringLiteral("message"), message}
    }}};
}

QJsonObject result(const QJsonObject &value)
{
    return {{QStringLiteral("result"), value}};
}

QJsonObject textResult(const QString &text, bool isError = false)
{
    return result({
        {QStringLiteral("content"), QJsonArray {QJsonObject {
            {QStringLiteral("type"), QStringLiteral("text")},
            {QStringLiteral("text"), text}
        }}},
        {QStringLiteral("isError"), isError}
    });
}

} // namespace

McpServer::McpServer(QObject *parent)
    : QObject(parent)
{
    mHttpServer.route(QStringLiteral("/mcp"), [this](const QHttpServerRequest &request) {
        return handleRequest(request);
    });
}

bool McpServer::start()
{
    if (mListener.isListening())
        return true;
    if (!mListener.listen(QHostAddress::LocalHost))
        return false;
    mHttpServer.bind(&mListener);
    mToken = QUuid::createUuid().toByteArray(QUuid::Id128);
    return true;
}

void McpServer::stop()
{
    mListener.close();
    mToken.clear();
}

QString McpServer::url() const
{
    return QStringLiteral("http://127.0.0.1:%1/mcp").arg(mListener.serverPort());
}

QHttpServerResponse McpServer::handleRequest(const QHttpServerRequest &request)
{
    using Status = QHttpServerResponder::StatusCode;

    // No browser origins are allowed. Validate Host as well to prevent DNS rebinding.
    if (!request.value("Origin").isEmpty()
            || request.value("Host") != QByteArray("127.0.0.1:") + QByteArray::number(mListener.serverPort()))
        return Status::Forbidden;
    if (mToken.isEmpty() || request.value("Authorization") != QByteArray("Bearer ") + mToken)
        return Status::Unauthorized;
    if (request.method() != QHttpServerRequest::Method::Post)
        return Status::MethodNotAllowed;
    if (request.value("Content-Type").split(';').first().trimmed().toLower() != "application/json")
        return Status::UnsupportedMediaType;
    if (!request.value("Accept").contains("application/json"))
        return Status::NotAcceptable;
    const auto version = request.value("MCP-Protocol-Version");
    if (!version.isEmpty() && !supportedVersion(QString::fromLatin1(version)))
        return Status::BadRequest;
    if (request.body().size() > 1024 * 1024)
        return Status::PayloadTooLarge;

    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(request.body(), &parseError);
    const auto message = document.object();
    const auto id = message.value(QStringLiteral("id"));
    QJsonObject response;
    if (parseError.error != QJsonParseError::NoError) {
        response = error(-32700, QStringLiteral("Parse error"));
    } else if (!document.isObject()
               || message.value(QStringLiteral("jsonrpc")) != QLatin1String("2.0")
               || !message.value(QStringLiteral("method")).isString()
               || (!id.isUndefined() && !id.isString() && !id.isDouble())
               || (message.contains(QStringLiteral("params")) && !message.value(QStringLiteral("params")).isObject())) {
        response = error(-32600, QStringLiteral("Invalid request"));
    } else if (id.isUndefined()) {
        // Notifications never execute tools, and do not produce JSON-RPC responses.
        return Status::Accepted;
    } else {
        response = dispatch(message);
    }
    response.insert(QStringLiteral("jsonrpc"), QStringLiteral("2.0"));
    response.insert(QStringLiteral("id"), id.isString() || id.isDouble() ? id : QJsonValue::Null);
    return QHttpServerResponse(response);
}

QJsonObject McpServer::dispatch(const QJsonObject &request)
{
    const auto method = request.value(QStringLiteral("method")).toString();
    const auto params = request.value(QStringLiteral("params")).toObject();
    if (method == QLatin1String("initialize")) {
        if (!params.value(QStringLiteral("protocolVersion")).isString()
                || !params.value(QStringLiteral("capabilities")).isObject()
                || !params.value(QStringLiteral("clientInfo")).isObject())
            return error(-32602, QStringLiteral("Invalid initialization parameters"));
        const auto requested = params.value(QStringLiteral("protocolVersion")).toString();
        return result({
            {QStringLiteral("protocolVersion"), supportedVersion(requested) ? requested : protocolVersion},
            {QStringLiteral("capabilities"), QJsonObject {
                {QStringLiteral("tools"), QJsonObject {}},
                {QStringLiteral("resources"), QJsonObject {}}
            }},
            {QStringLiteral("serverInfo"), QJsonObject {
                {QStringLiteral("name"), QStringLiteral("Tiled")},
                {QStringLiteral("version"), QCoreApplication::applicationVersion()}
            }},
            {QStringLiteral("instructions"), QStringLiteral(
                "Read tiled://scripting-api before editing. Work with the live documents, including unsaved changes. "
                "Group edits with asset.macro() for Undo. Save only when requested. Scripts have the same "
                "file and process access as local extensions. Script errors do not roll back preceding edits.")}
        });
    }
    if (method == QLatin1String("ping"))
        return result({});
    if (method == QLatin1String("tools/list")) {
        return result({{QStringLiteral("tools"), QJsonArray {QJsonObject {
            {QStringLiteral("name"), QStringLiteral("execute_script")},
            {QStringLiteral("description"), QStringLiteral(
                "Execute JavaScript in the running Tiled editor using its scripting API. "
                "code is a synchronous function body; return a JSON-serializable value. "
                "For example: return tiled.openAssets.map(a => ({fileName: a.fileName, modified: a.modified})); "
                "Read tiled://scripting-api for the API, including tiled.activeAsset and asset.macro(). "
                "Use existing API operations for edits and explicit save() calls only when requested.")},
            {QStringLiteral("inputSchema"), QJsonObject {
                {QStringLiteral("type"), QStringLiteral("object")},
                {QStringLiteral("properties"), QJsonObject {
                    {QStringLiteral("code"), QJsonObject {{QStringLiteral("type"), QStringLiteral("string")}}}
                }},
                {QStringLiteral("required"), QJsonArray {QStringLiteral("code")}},
                {QStringLiteral("additionalProperties"), false}
            }}
        }}}});
    }
    if (method == QLatin1String("tools/call")) {
        const auto arguments = params.value(QStringLiteral("arguments")).toObject();
        if (params.value(QStringLiteral("name")) != QLatin1String("execute_script"))
            return error(-32602, QStringLiteral("Unknown tool"));
        if (arguments.size() != 1 || !arguments.value(QStringLiteral("code")).isString())
            return error(-32602, QStringLiteral("Expected a code string"));
        return executeScript(arguments.value(QStringLiteral("code")).toString());
    }
    if (method == QLatin1String("resources/list")) {
        return result({{QStringLiteral("resources"), QJsonArray {QJsonObject {
            {QStringLiteral("uri"), apiUri},
            {QStringLiteral("name"), QStringLiteral("Tiled scripting API")},
            {QStringLiteral("mimeType"), QStringLiteral("text/plain")}
        }}}});
    }
    if (method == QLatin1String("resources/read")) {
        if (params.value(QStringLiteral("uri")) != apiUri)
            return error(-32002, QStringLiteral("Unknown resource"));
        QFile file(QStringLiteral(":/mcp/scripting-api.d.ts"));
        if (!file.open(QFile::ReadOnly))
            return error(-32603, QStringLiteral("Could not read scripting API"));
        return result({{QStringLiteral("contents"), QJsonArray {QJsonObject {
            {QStringLiteral("uri"), apiUri},
            {QStringLiteral("mimeType"), QStringLiteral("text/plain")},
            {QStringLiteral("text"), QString::fromUtf8(file.readAll())}
        }}}});
    }
    return error(-32601, QStringLiteral("Method not found"));
}

QJsonObject McpServer::executeScript(const QString &code)
{
    // A script can enter a nested event loop (for example, by showing a dialog).
    if (mBusy)
        return textResult(QStringLiteral("Another MCP script is still running"), true);
    QScopedValueRollback<bool> busy(mBusy, true);
    ScriptManager::ResetBlocker resetBlocker;
    auto &manager = ScriptManager::instance();
    manager.ensureInitialized();
    const auto script = QStringLiteral("JSON.stringify((function() {\n%1\n})())").arg(code);
    QStringList stack;
    const auto value = manager.engine()->evaluate(script, QStringLiteral("mcp"), 1, &stack);
    if (!stack.isEmpty() || value.isError())
        return textResult(value.toString(), true);
    return textResult(value.isUndefined() ? QStringLiteral("null") : value.toString());
}

void McpServer::showDialog(QWidget *parent)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(tr("MCP Server"));
    auto layout = new QVBoxLayout(&dialog);
    auto label = new QLabel(tr("Allow an MCP client to run scripts in this editor.\n"
                              "Connected clients have the same access as local extensions."));
    layout->addWidget(label);
    auto enabled = new QCheckBox(tr("Enable MCP server"));
    enabled->setChecked(mListener.isListening());
    layout->addWidget(enabled);
    auto copy = new QPushButton(tr("Copy Client Configuration"));
    copy->setEnabled(enabled->isChecked());
    layout->addWidget(copy);
    connect(enabled, &QCheckBox::toggled, &dialog, [this, &dialog, enabled, copy](bool checked) {
        if (checked && !start()) {
            QMessageBox::warning(&dialog, tr("MCP Server"), mListener.errorString());
            enabled->setChecked(false);
        } else if (!checked) {
            stop();
        }
        copy->setEnabled(mListener.isListening());
    });
    connect(copy, &QPushButton::clicked, &dialog, [this] {
        const QJsonObject config {{QStringLiteral("mcpServers"), QJsonObject {
            {QStringLiteral("tiled"), QJsonObject {
                {QStringLiteral("url"), url()},
                {QStringLiteral("headers"), QJsonObject {
                    {QStringLiteral("Authorization"), QString::fromLatin1("Bearer " + mToken)}
                }}
            }}
        }}};
        QGuiApplication::clipboard()->setText(QString::fromUtf8(QJsonDocument(config).toJson()));
    });
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    dialog.exec();
}

} // namespace Tiled
