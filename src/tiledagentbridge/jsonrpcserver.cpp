#include "jsonrpcserver.h"

#include <QJsonDocument>

namespace tiledagent {

JsonRpcServer::JsonRpcServer(BridgeEngine &engine, QTextStream *input, QTextStream *output)
    : mEngine(engine)
    , mInput(input)
    , mOutput(output)
{
}

QJsonObject JsonRpcServer::makeErrorResponse(const QJsonValue &id, const QString &code, const QString &message) const
{
    return QJsonObject{
        { QStringLiteral("jsonrpc"), QStringLiteral("2.0") },
        { QStringLiteral("id"), id },
        { QStringLiteral("error"), QJsonObject{
            { QStringLiteral("code"), code },
            { QStringLiteral("message"), message },
        } }
    };
}

QJsonObject JsonRpcServer::makeSuccessResponse(const QJsonValue &id, const QJsonObject &result) const
{
    return QJsonObject{
        { QStringLiteral("jsonrpc"), QStringLiteral("2.0") },
        { QStringLiteral("id"), id },
        { QStringLiteral("result"), result }
    };
}

QJsonObject JsonRpcServer::dispatch(const QJsonObject &request)
{
    const QJsonValue id = request.value(QStringLiteral("id"));
    const QString method = request.value(QStringLiteral("method")).toString();
    const auto params = request.value(QStringLiteral("params")).toObject();
    const QString sessionId = params.value(QStringLiteral("sessionId")).toString();

    if (method == QStringLiteral("openDocument")) {
        const auto result = mEngine.openDocument(sessionId, params.value(QStringLiteral("documentPath")).toString());
        if (!result.success)
            return makeErrorResponse(id, result.errorCode, result.errorMessage);
        return makeSuccessResponse(id, QJsonObject{
            { QStringLiteral("documentType"), result.documentType },
            { QStringLiteral("revision"), result.revision },
            { QStringLiteral("snapshot"), result.snapshot },
        });
    }

    if (method == QStringLiteral("getSnapshot")) {
        const auto result = mEngine.getSnapshot(sessionId);
        if (!result.success)
            return makeErrorResponse(id, result.errorCode, result.errorMessage);
        return makeSuccessResponse(id, QJsonObject{
            { QStringLiteral("documentType"), result.documentType },
            { QStringLiteral("revision"), result.revision },
            { QStringLiteral("snapshot"), result.snapshot },
        });
    }

    if (method == QStringLiteral("executeCommand")) {
        const auto result = mEngine.executeCommand(sessionId, params.value(QStringLiteral("command")).toObject());
        if (!result.success)
            return makeErrorResponse(id, result.errorCode, result.errorMessage);
        return makeSuccessResponse(id, QJsonObject{
            { QStringLiteral("revisionBefore"), result.revisionBefore },
            { QStringLiteral("revisionAfter"), result.revisionAfter },
            { QStringLiteral("changedEntities"), result.changedEntities },
            { QStringLiteral("warnings"), result.warnings },
            { QStringLiteral("errors"), result.errors },
            { QStringLiteral("undoDepth"), result.undoDepth },
            { QStringLiteral("redoDepth"), result.redoDepth },
        });
    }

    if (method == QStringLiteral("validateDocument")) {
        const auto result = mEngine.validateDocument(sessionId);
        if (!result.success)
            return makeErrorResponse(id, result.errorCode, result.errorMessage);
        return makeSuccessResponse(id, QJsonObject{
            { QStringLiteral("revision"), result.revision },
            { QStringLiteral("diagnostics"), result.diagnostics },
        });
    }

    if (method == QStringLiteral("saveDocument")) {
        const auto result = mEngine.saveDocument(sessionId);
        if (!result.success)
            return makeErrorResponse(id, result.errorCode, result.errorMessage);
        return makeSuccessResponse(id, QJsonObject{
            { QStringLiteral("path"), result.path },
            { QStringLiteral("revision"), result.revision },
        });
    }

    if (method == QStringLiteral("undo")) {
        const auto result = mEngine.undo(sessionId);
        if (!result.success)
            return makeErrorResponse(id, result.errorCode, result.errorMessage);
        return makeSuccessResponse(id, QJsonObject{
            { QStringLiteral("revisionBefore"), result.revisionBefore },
            { QStringLiteral("revisionAfter"), result.revisionAfter },
            { QStringLiteral("changedEntities"), result.changedEntities },
            { QStringLiteral("warnings"), result.warnings },
            { QStringLiteral("errors"), result.errors },
            { QStringLiteral("undoDepth"), result.undoDepth },
            { QStringLiteral("redoDepth"), result.redoDepth },
        });
    }

    if (method == QStringLiteral("redo")) {
        const auto result = mEngine.redo(sessionId);
        if (!result.success)
            return makeErrorResponse(id, result.errorCode, result.errorMessage);
        return makeSuccessResponse(id, QJsonObject{
            { QStringLiteral("revisionBefore"), result.revisionBefore },
            { QStringLiteral("revisionAfter"), result.revisionAfter },
            { QStringLiteral("changedEntities"), result.changedEntities },
            { QStringLiteral("warnings"), result.warnings },
            { QStringLiteral("errors"), result.errors },
            { QStringLiteral("undoDepth"), result.undoDepth },
            { QStringLiteral("redoDepth"), result.redoDepth },
        });
    }

    if (method == QStringLiteral("closeSession")) {
        return makeSuccessResponse(id, QJsonObject{
            { QStringLiteral("closed"), mEngine.closeSession(sessionId) },
        });
    }

    return makeErrorResponse(id, QStringLiteral("METHOD_NOT_FOUND"), QStringLiteral("Unknown method: %1").arg(method));
}

int JsonRpcServer::run()
{
    QTextStream stdIn(stdin);
    QTextStream stdOut(stdout);
    QTextStream &input = mInput ? *mInput : stdIn;
    QTextStream &output = mOutput ? *mOutput : stdOut;

    while (!input.atEnd()) {
        const QString line = input.readLine().trimmed();
        if (line.isEmpty())
            continue;

        QJsonParseError parseError;
        const auto document = QJsonDocument::fromJson(line.toUtf8(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            output << QJsonDocument(makeErrorResponse(QJsonValue(), QStringLiteral("PARSE_ERROR"), parseError.errorString())).toJson(QJsonDocument::Compact) << '\n';
            output.flush();
            continue;
        }

        output << QJsonDocument(dispatch(document.object())).toJson(QJsonDocument::Compact) << '\n';
        output.flush();
    }

    return 0;
}

} // namespace tiledagent
