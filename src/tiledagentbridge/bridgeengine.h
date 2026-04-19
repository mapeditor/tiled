#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QHash>
#include <QSharedPointer>
#include <QString>
#include <QVariant>
#include <QVector>

#include <memory>

namespace Tiled {
class Map;
class Tileset;
using SharedTileset = QSharedPointer<Tileset>;
}

namespace tiledagent {

struct SnapshotResult {
    bool success = false;
    QString errorCode;
    QString errorMessage;
    QString documentType;
    int revision = -1;
    QJsonObject snapshot;
};

struct CommandResult {
    bool success = false;
    QString errorCode;
    QString errorMessage;
    int revisionBefore = -1;
    int revisionAfter = -1;
    QJsonArray changedEntities;
    QJsonArray warnings;
    QJsonArray errors;
    int undoDepth = 0;
    int redoDepth = 0;
};

struct ValidationResult {
    bool success = false;
    QString errorCode;
    QString errorMessage;
    int revision = -1;
    QJsonArray diagnostics;
};

struct SaveResult {
    bool success = false;
    QString errorCode;
    QString errorMessage;
    QString path;
    int revision = -1;
};

class BridgeEngine
{
public:
    BridgeEngine();

    SnapshotResult openDocument(const QString &sessionId, const QString &documentPath);
    SnapshotResult getSnapshot(const QString &sessionId) const;
    CommandResult executeCommand(const QString &sessionId, const QJsonObject &command);
    ValidationResult validateDocument(const QString &sessionId) const;
    SaveResult saveDocument(const QString &sessionId);
    CommandResult undo(const QString &sessionId);
    CommandResult redo(const QString &sessionId);
    bool closeSession(const QString &sessionId);

private:
    enum class DocumentKind {
        Map,
        Tileset,
    };

    struct SerializedState {
        DocumentKind kind;
        QVariant variant;
    };

    struct SessionState {
        QString path;
        DocumentKind kind = DocumentKind::Map;
        std::unique_ptr<Tiled::Map> map;
        Tiled::SharedTileset tileset;
        int revision = 0;
        QVector<SerializedState> undoStack;
        QVector<SerializedState> redoStack;
    };

    SessionState *session(const QString &sessionId);
    const SessionState *session(const QString &sessionId) const;

    SerializedState serializeState(const SessionState &session) const;
    bool restoreState(SessionState &session, const SerializedState &state, QString *errorMessage) const;
    static bool hasLoadedDocument(const SessionState &session);

    SnapshotResult snapshotResultFromSession(const SessionState &session) const;
    CommandResult commandFailure(const SessionState *session, const QString &code, const QString &message) const;
    ValidationResult validationFailure(const SessionState *session, const QString &code, const QString &message) const;
    SaveResult saveFailure(const SessionState *session, const QString &code, const QString &message) const;

    bool applyCommand(SessionState &session, const QJsonObject &command, QString *errorMessage, QJsonArray *changedEntities) const;

    QHash<QString, std::shared_ptr<SessionState>> mSessions;
};

} // namespace tiledagent
