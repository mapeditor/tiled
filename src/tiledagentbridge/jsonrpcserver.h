#pragma once

#include "bridgeengine.h"

#include <QTextStream>

namespace tiledagent {

class JsonRpcServer
{
public:
    explicit JsonRpcServer(BridgeEngine &engine, QTextStream *input = nullptr, QTextStream *output = nullptr);

    int run();

private:
    QJsonObject dispatch(const QJsonObject &request);
    QJsonObject makeErrorResponse(const QJsonValue &id, const QString &code, const QString &message) const;
    QJsonObject makeSuccessResponse(const QJsonValue &id, const QJsonObject &result) const;

    BridgeEngine &mEngine;
    QTextStream *mInput;
    QTextStream *mOutput;
};

} // namespace tiledagent
