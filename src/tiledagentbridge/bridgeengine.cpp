#include "bridgeengine.h"

#include "grouplayer.h"
#include "layer.h"
#include "map.h"
#include "mapobject.h"
#include "mapreader.h"
#include "maptovariantconverter.h"
#include "mapwriter.h"
#include "objectgroup.h"
#include "savefile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "varianttomapconverter.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSaveFile>
#include <QSet>

#include <algorithm>

namespace {

using tiledagent::BridgeEngine;

QVariant toVariant(const QJsonObject &object)
{
    return QJsonDocument(object).toVariant();
}

QJsonObject wrapSnapshot(const QJsonObject &payload, const QString &documentType)
{
    QJsonObject snapshot;
    snapshot.insert(QStringLiteral("documentType"), documentType);
    snapshot.insert(documentType, payload);
    return snapshot;
}

QJsonObject toJsonObject(const QVariant &variant)
{
    return QJsonDocument::fromVariant(variant).object();
}

QJsonArray stringArray(const QStringList &values)
{
    QJsonArray result;
    for (const QString &value : values)
        result.append(value);
    return result;
}

QJsonObject changedEntity(const QString &entityType, int entityId, const QString &changeType)
{
    return QJsonObject{
        { QStringLiteral("entityType"), entityType },
        { QStringLiteral("entityId"), entityId },
        { QStringLiteral("changeType"), changeType },
    };
}

Tiled::Cell cellForGid(const Tiled::Map &map, int gid, QString *errorMessage)
{
    if (gid == 0)
        return Tiled::Cell::empty;

    int firstGid = 1;
    for (const Tiled::SharedTileset &tileset : map.tilesets()) {
        const int nextFirstGid = firstGid + tileset->nextTileId();
        if (gid >= firstGid && gid < nextFirstGid) {
            const int tileId = gid - firstGid;
            if (!tileset->findTile(tileId)) {
                if (errorMessage)
                    *errorMessage = QStringLiteral("Unknown gid: %1").arg(gid);
                return Tiled::Cell::empty;
            }
            return Tiled::Cell(tileset.data(), tileId);
        }
        firstGid = nextFirstGid;
    }

    if (errorMessage)
        *errorMessage = QStringLiteral("Unknown gid: %1").arg(gid);
    return Tiled::Cell::empty;
}

Tiled::Properties propertiesFromJson(const QJsonObject &json)
{
    Tiled::Properties properties;
    for (auto it = json.begin(); it != json.end(); ++it)
        properties.insert(it.key(), it.value().toVariant());
    return properties;
}

QJsonObject tilesetSnapshot(const Tiled::Tileset &tileset)
{
    QJsonObject object;
    object.insert(QStringLiteral("name"), tileset.name());
    object.insert(QStringLiteral("tileWidth"), tileset.tileWidth());
    object.insert(QStringLiteral("tileHeight"), tileset.tileHeight());
    object.insert(QStringLiteral("tileCount"), tileset.tileCount());
    object.insert(QStringLiteral("columns"), tileset.columnCount());
    if (!tileset.fileName().isEmpty())
        object.insert(QStringLiteral("source"), tileset.fileName());
    if (!tileset.imageSource().isEmpty())
        object.insert(QStringLiteral("image"), tileset.imageSource().toString());
    return object;
}

QJsonObject mapSnapshot(const Tiled::Map &map, const QString &path)
{
    Tiled::MapToVariantConverter converter;
    const auto variant = converter.toVariant(map, QFileInfo(path).dir());
    return toJsonObject(variant);
}

bool saveJsonVariant(const QVariant &variant, const QString &path, QString *errorMessage)
{
    Tiled::SaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }

    const auto json = QJsonDocument::fromVariant(variant).toJson(QJsonDocument::Indented);
    if (file.device()->write(json) != json.size()) {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }

    if (!file.commit()) {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }

    return true;
}

} // namespace

namespace tiledagent {

BridgeEngine::BridgeEngine() = default;

BridgeEngine::SessionState *BridgeEngine::session(const QString &sessionId)
{
    const auto it = mSessions.find(sessionId);
    if (it == mSessions.end())
        return nullptr;
    return it.value().get();
}

const BridgeEngine::SessionState *BridgeEngine::session(const QString &sessionId) const
{
    const auto it = mSessions.constFind(sessionId);
    if (it == mSessions.constEnd())
        return nullptr;
    return it.value().get();
}

BridgeEngine::SerializedState BridgeEngine::serializeState(const SessionState &session) const
{
    Tiled::MapToVariantConverter converter;
    const QDir directory = QFileInfo(session.path).dir();

    if (session.kind == DocumentKind::Map)
        return { session.kind, converter.toVariant(*session.map, directory) };

    return { session.kind, converter.toVariant(*session.tileset, directory) };
}

bool BridgeEngine::restoreState(SessionState &session, const SerializedState &state, QString *errorMessage) const
{
    Tiled::VariantToMapConverter converter;
    const QDir directory = QFileInfo(session.path).dir();

    if (state.kind == DocumentKind::Map) {
        auto map = converter.toMap(state.variant, directory);
        if (!map) {
            if (errorMessage)
                *errorMessage = converter.errorString();
            return false;
        }
        map->fileName = session.path;
        session.kind = DocumentKind::Map;
        session.map = std::move(map);
        session.tileset.clear();
        return true;
    }

    auto tileset = converter.toTileset(state.variant, directory);
    if (!tileset) {
        if (errorMessage)
            *errorMessage = converter.errorString();
        return false;
    }
    tileset->setFileName(session.path);
    session.kind = DocumentKind::Tileset;
    session.tileset = tileset;
    session.map.reset();
    return true;
}

bool BridgeEngine::hasLoadedDocument(const SessionState &session)
{
    return (session.kind == DocumentKind::Map && session.map != nullptr)
        || (session.kind == DocumentKind::Tileset && !session.tileset.isNull());
}

SnapshotResult BridgeEngine::snapshotResultFromSession(const SessionState &session) const
{
    SnapshotResult result;
    result.success = true;
    result.revision = session.revision;
    result.documentType = session.kind == DocumentKind::Map ? QStringLiteral("map") : QStringLiteral("tileset");
    if (session.kind == DocumentKind::Map)
        result.snapshot = wrapSnapshot(mapSnapshot(*session.map, session.path), result.documentType);
    else
        result.snapshot = wrapSnapshot(tilesetSnapshot(*session.tileset), result.documentType);
    return result;
}

CommandResult BridgeEngine::commandFailure(const SessionState *session, const QString &code, const QString &message) const
{
    CommandResult result;
    result.success = false;
    result.errorCode = code;
    result.errorMessage = message;
    if (session) {
        result.revisionBefore = session->revision;
        result.revisionAfter = session->revision;
        result.undoDepth = session->undoStack.size();
        result.redoDepth = session->redoStack.size();
    }
    result.errors = stringArray(QStringList{ message });
    return result;
}

ValidationResult BridgeEngine::validationFailure(const SessionState *session, const QString &code, const QString &message) const
{
    ValidationResult result;
    result.success = false;
    result.errorCode = code;
    result.errorMessage = message;
    if (session)
        result.revision = session->revision;
    result.diagnostics.append(QJsonObject{
        { QStringLiteral("severity"), QStringLiteral("error") },
        { QStringLiteral("code"), code },
        { QStringLiteral("message"), message },
    });
    return result;
}

SaveResult BridgeEngine::saveFailure(const SessionState *session, const QString &code, const QString &message) const
{
    SaveResult result;
    result.success = false;
    result.errorCode = code;
    result.errorMessage = message;
    if (session)
        result.revision = session->revision;
    return result;
}

SnapshotResult BridgeEngine::openDocument(const QString &sessionId, const QString &documentPath)
{
    const QFileInfo fileInfo(documentPath);
    const QString suffix = fileInfo.suffix().toLower();

    SessionState state;
    state.path = fileInfo.absoluteFilePath();
    state.revision = 0;

    if (suffix == QStringLiteral("tmx")) {
        Tiled::MapReader reader;
        state.map = reader.readMap(state.path);
        if (!state.map) {
            SnapshotResult result;
            result.errorCode = QStringLiteral("DOCUMENT_ERROR");
            result.errorMessage = reader.errorString();
            return result;
        }
        state.map->fileName = state.path;
        state.kind = DocumentKind::Map;
    } else if (suffix == QStringLiteral("tmj") || suffix == QStringLiteral("json")) {
        QFile file(state.path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            SnapshotResult result;
            result.errorCode = QStringLiteral("DOCUMENT_ERROR");
            result.errorMessage = file.errorString();
            return result;
        }

        QJsonParseError parseError;
        const auto document = QJsonDocument::fromJson(file.readAll(), &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            SnapshotResult result;
            result.errorCode = QStringLiteral("DOCUMENT_ERROR");
            result.errorMessage = parseError.errorString();
            return result;
        }

        const auto object = document.object();
        Tiled::VariantToMapConverter converter;
        if (object.value(QStringLiteral("type")).toString() == QStringLiteral("tileset")) {
            state.tileset = converter.toTileset(document.toVariant(), fileInfo.dir());
            if (!state.tileset) {
                SnapshotResult result;
                result.errorCode = QStringLiteral("DOCUMENT_ERROR");
                result.errorMessage = converter.errorString();
                return result;
            }
            state.tileset->setFileName(state.path);
            state.kind = DocumentKind::Tileset;
        } else {
            state.map = converter.toMap(document.toVariant(), fileInfo.dir());
            if (!state.map) {
                SnapshotResult result;
                result.errorCode = QStringLiteral("DOCUMENT_ERROR");
                result.errorMessage = converter.errorString();
                return result;
            }
            state.map->fileName = state.path;
            state.kind = DocumentKind::Map;
        }
    } else if (suffix == QStringLiteral("tsx") || suffix == QStringLiteral("tsj")) {
        if (suffix == QStringLiteral("tsx")) {
            Tiled::MapReader reader;
            state.tileset = reader.readTileset(state.path);
            if (!state.tileset) {
                SnapshotResult result;
                result.errorCode = QStringLiteral("DOCUMENT_ERROR");
                result.errorMessage = reader.errorString();
                return result;
            }
            state.tileset->setFileName(state.path);
            state.kind = DocumentKind::Tileset;
        } else {
            QFile file(state.path);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                SnapshotResult result;
                result.errorCode = QStringLiteral("DOCUMENT_ERROR");
                result.errorMessage = file.errorString();
                return result;
            }

            QJsonParseError parseError;
            const auto document = QJsonDocument::fromJson(file.readAll(), &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                SnapshotResult result;
                result.errorCode = QStringLiteral("DOCUMENT_ERROR");
                result.errorMessage = parseError.errorString();
                return result;
            }

            Tiled::VariantToMapConverter converter;
            state.tileset = converter.toTileset(document.toVariant(), fileInfo.dir());
            if (!state.tileset) {
                SnapshotResult result;
                result.errorCode = QStringLiteral("DOCUMENT_ERROR");
                result.errorMessage = converter.errorString();
                return result;
            }
            state.tileset->setFileName(state.path);
            state.kind = DocumentKind::Tileset;
        }
    } else {
        SnapshotResult result;
        result.errorCode = QStringLiteral("DOCUMENT_ERROR");
        result.errorMessage = QStringLiteral("Unsupported document extension: %1").arg(suffix);
        return result;
    }

    mSessions.insert(sessionId, std::make_shared<SessionState>(std::move(state)));
    return snapshotResultFromSession(*session(sessionId));
}

SnapshotResult BridgeEngine::getSnapshot(const QString &sessionId) const
{
    const auto currentSession = session(sessionId);
    if (!currentSession) {
        SnapshotResult result;
        result.errorCode = QStringLiteral("SESSION_NOT_FOUND");
        result.errorMessage = QStringLiteral("Unknown session: %1").arg(sessionId);
        return result;
    }
    if (!hasLoadedDocument(*currentSession)) {
        SnapshotResult result;
        result.errorCode = QStringLiteral("DOCUMENT_NOT_OPEN");
        result.errorMessage = QStringLiteral("No document is open for session: %1").arg(sessionId);
        return result;
    }
    return snapshotResultFromSession(*currentSession);
}

bool BridgeEngine::applyCommand(SessionState &session, const QJsonObject &command, QString *errorMessage, QJsonArray *changedEntities) const
{
    if (session.kind != DocumentKind::Map) {
        if (errorMessage)
            *errorMessage = QStringLiteral("Commands are only supported for map documents.");
        return false;
    }

    const QString type = command.value(QStringLiteral("type")).toString();
    const auto payload = command.value(QStringLiteral("payload")).toObject();
    auto map = session.map->clone();

    if (type == QStringLiteral("create_layer")) {
        const QString layerType = payload.value(QStringLiteral("layerType")).toString();
        const QString name = payload.value(QStringLiteral("name")).toString();
        std::unique_ptr<Tiled::Layer> layer;

        if (layerType == QStringLiteral("tile")) {
            layer = std::make_unique<Tiled::TileLayer>(name, 0, 0, map->width(), map->height());
        } else if (layerType == QStringLiteral("object")) {
            layer = std::make_unique<Tiled::ObjectGroup>(name);
        } else if (layerType == QStringLiteral("group")) {
            layer = std::make_unique<Tiled::GroupLayer>(name, 0, 0);
        } else {
            if (errorMessage)
                *errorMessage = QStringLiteral("Unsupported layer type: %1").arg(layerType);
            return false;
        }

        layer->setId(map->takeNextLayerId());
        const int layerId = layer->id();
        map->addLayer(std::move(layer));
        session.undoStack.append(serializeState(session));
        session.redoStack.clear();
        session.map = std::move(map);
        changedEntities->append(changedEntity(QStringLiteral("layer"), layerId, QStringLiteral("created")));
        return true;
    }

    if (type == QStringLiteral("delete_layer")) {
        const int layerId = payload.value(QStringLiteral("layerId")).toInt();
        for (int index = 0; index < map->layerCount(); ++index) {
            if (map->layerAt(index)->id() == layerId) {
                std::unique_ptr<Tiled::Layer> removed(map->takeLayerAt(index));
                session.undoStack.append(serializeState(session));
                session.redoStack.clear();
                session.map = std::move(map);
                changedEntities->append(changedEntity(QStringLiteral("layer"), layerId, QStringLiteral("deleted")));
                return true;
            }
        }
        if (errorMessage)
            *errorMessage = QStringLiteral("Unknown layer id: %1").arg(layerId);
        return false;
    }

    if (type == QStringLiteral("move_layer")) {
        const int layerId = payload.value(QStringLiteral("layerId")).toInt();
        const int newIndex = payload.value(QStringLiteral("index")).toInt(-1);
        if (newIndex < 0 || newIndex >= map->layerCount()) {
            if (errorMessage)
                *errorMessage = QStringLiteral("Invalid layer index: %1").arg(newIndex);
            return false;
        }

        for (int index = 0; index < map->layerCount(); ++index) {
            if (map->layerAt(index)->id() == layerId) {
                std::unique_ptr<Tiled::Layer> moved(map->takeLayerAt(index));
                map->insertLayer(newIndex, moved.release());
                session.undoStack.append(serializeState(session));
                session.redoStack.clear();
                session.map = std::move(map);
                changedEntities->append(changedEntity(QStringLiteral("layer"), layerId, QStringLiteral("moved")));
                return true;
            }
        }

        if (errorMessage)
            *errorMessage = QStringLiteral("Unknown layer id: %1").arg(layerId);
        return false;
    }

    if (type == QStringLiteral("set_layer_properties")) {
        const int layerId = payload.value(QStringLiteral("layerId")).toInt();
        auto *layer = map->findLayerById(layerId);
        if (!layer) {
            if (errorMessage)
                *errorMessage = QStringLiteral("Unknown layer id: %1").arg(layerId);
            return false;
        }

        if (payload.contains(QStringLiteral("name")))
            layer->setName(payload.value(QStringLiteral("name")).toString());
        if (payload.contains(QStringLiteral("visible")))
            layer->setVisible(payload.value(QStringLiteral("visible")).toBool());
        if (payload.contains(QStringLiteral("opacity")))
            layer->setOpacity(payload.value(QStringLiteral("opacity")).toDouble());
        if (payload.contains(QStringLiteral("x")))
            layer->setX(payload.value(QStringLiteral("x")).toInt());
        if (payload.contains(QStringLiteral("y")))
            layer->setY(payload.value(QStringLiteral("y")).toInt());
        if (payload.contains(QStringLiteral("properties")))
            layer->mergeProperties(propertiesFromJson(payload.value(QStringLiteral("properties")).toObject()));

        session.undoStack.append(serializeState(session));
        session.redoStack.clear();
        session.map = std::move(map);
        changedEntities->append(changedEntity(QStringLiteral("layer"), layerId, QStringLiteral("updated")));
        return true;
    }

    if (type == QStringLiteral("paint_tiles") || type == QStringLiteral("erase_tiles")) {
        const int layerId = payload.value(QStringLiteral("layerId")).toInt();
        auto *layer = dynamic_cast<Tiled::TileLayer*>(map->findLayerById(layerId));
        if (!layer) {
            if (errorMessage)
                *errorMessage = QStringLiteral("Unknown tile layer id: %1").arg(layerId);
            return false;
        }

        const auto cells = payload.value(QStringLiteral("cells")).toArray();
        for (const auto &cellValue : cells) {
            const auto cellObject = cellValue.toObject();
            const int x = cellObject.value(QStringLiteral("x")).toInt();
            const int y = cellObject.value(QStringLiteral("y")).toInt();

            if (!map->infinite() && !layer->contains(x, y)) {
                if (errorMessage)
                    *errorMessage = QStringLiteral("Cell (%1, %2) is outside layer bounds.").arg(x).arg(y);
                return false;
            }

            if (type == QStringLiteral("erase_tiles")) {
                layer->setCell(x, y, Tiled::Cell::empty);
                continue;
            }

            QString gidError;
            const auto cell = cellForGid(*map, cellObject.value(QStringLiteral("gid")).toInt(), &gidError);
            if (!gidError.isEmpty()) {
                if (errorMessage)
                    *errorMessage = gidError;
                return false;
            }
            layer->setCell(x, y, cell);
        }

        session.undoStack.append(serializeState(session));
        session.redoStack.clear();
        session.map = std::move(map);
        changedEntities->append(changedEntity(QStringLiteral("layer"), layerId, QStringLiteral("updated")));
        return true;
    }

    if (type == QStringLiteral("create_object")) {
        const int layerId = payload.value(QStringLiteral("layerId")).toInt();
        auto *group = dynamic_cast<Tiled::ObjectGroup*>(map->findLayerById(layerId));
        if (!group) {
            if (errorMessage)
                *errorMessage = QStringLiteral("Unknown object layer id: %1").arg(layerId);
            return false;
        }

        auto object = std::make_unique<Tiled::MapObject>(
            payload.value(QStringLiteral("name")).toString(),
            payload.value(QStringLiteral("className")).toString(),
            QPointF(payload.value(QStringLiteral("x")).toDouble(), payload.value(QStringLiteral("y")).toDouble()),
            QSizeF(payload.value(QStringLiteral("width")).toDouble(), payload.value(QStringLiteral("height")).toDouble())
        );
        object->setId(map->takeNextObjectId());
        if (payload.contains(QStringLiteral("rotation")))
            object->setRotation(payload.value(QStringLiteral("rotation")).toDouble());
        if (payload.contains(QStringLiteral("visible")))
            object->setVisible(payload.value(QStringLiteral("visible")).toBool());
        if (payload.contains(QStringLiteral("properties")))
            object->mergeProperties(propertiesFromJson(payload.value(QStringLiteral("properties")).toObject()));

        const int objectId = object->id();
        group->addObject(std::move(object));
        session.undoStack.append(serializeState(session));
        session.redoStack.clear();
        session.map = std::move(map);
        changedEntities->append(changedEntity(QStringLiteral("object"), objectId, QStringLiteral("created")));
        return true;
    }

    if (type == QStringLiteral("update_object") || type == QStringLiteral("move_object")) {
        const int objectId = payload.value(QStringLiteral("objectId")).toInt();
        auto *object = map->findObjectById(objectId);
        if (!object) {
            if (errorMessage)
                *errorMessage = QStringLiteral("Unknown object id: %1").arg(objectId);
            return false;
        }

        if (payload.contains(QStringLiteral("name")))
            object->setName(payload.value(QStringLiteral("name")).toString());
        if (payload.contains(QStringLiteral("className")))
            object->setClassName(payload.value(QStringLiteral("className")).toString());
        if (payload.contains(QStringLiteral("x")))
            object->setX(payload.value(QStringLiteral("x")).toDouble());
        if (payload.contains(QStringLiteral("y")))
            object->setY(payload.value(QStringLiteral("y")).toDouble());
        if (payload.contains(QStringLiteral("width")))
            object->setWidth(payload.value(QStringLiteral("width")).toDouble());
        if (payload.contains(QStringLiteral("height")))
            object->setHeight(payload.value(QStringLiteral("height")).toDouble());
        if (payload.contains(QStringLiteral("rotation")))
            object->setRotation(payload.value(QStringLiteral("rotation")).toDouble());
        if (payload.contains(QStringLiteral("visible")))
            object->setVisible(payload.value(QStringLiteral("visible")).toBool());
        if (payload.contains(QStringLiteral("properties")))
            object->mergeProperties(propertiesFromJson(payload.value(QStringLiteral("properties")).toObject()));

        session.undoStack.append(serializeState(session));
        session.redoStack.clear();
        session.map = std::move(map);
        changedEntities->append(changedEntity(QStringLiteral("object"), objectId, QStringLiteral("updated")));
        return true;
    }

    if (type == QStringLiteral("delete_object")) {
        const int objectId = payload.value(QStringLiteral("objectId")).toInt();
        auto *object = map->findObjectById(objectId);
        if (!object || !object->objectGroup()) {
            if (errorMessage)
                *errorMessage = QStringLiteral("Unknown object id: %1").arg(objectId);
            return false;
        }

        std::unique_ptr<Tiled::MapObject> removed(object);
        object->objectGroup()->removeObject(object);
        session.undoStack.append(serializeState(session));
        session.redoStack.clear();
        session.map = std::move(map);
        changedEntities->append(changedEntity(QStringLiteral("object"), objectId, QStringLiteral("deleted")));
        return true;
    }

    if (type == QStringLiteral("set_custom_properties")) {
        const QString targetType = payload.value(QStringLiteral("targetType")).toString();
        const auto properties = propertiesFromJson(payload.value(QStringLiteral("properties")).toObject());

        if (targetType == QStringLiteral("map")) {
            map->mergeProperties(properties);
            session.undoStack.append(serializeState(session));
            session.redoStack.clear();
            session.map = std::move(map);
            changedEntities->append(changedEntity(QStringLiteral("map"), 0, QStringLiteral("updated")));
            return true;
        }

        if (targetType == QStringLiteral("layer")) {
            const int layerId = payload.value(QStringLiteral("targetId")).toInt();
            auto *layer = map->findLayerById(layerId);
            if (!layer) {
                if (errorMessage)
                    *errorMessage = QStringLiteral("Unknown layer id: %1").arg(layerId);
                return false;
            }
            layer->mergeProperties(properties);
            session.undoStack.append(serializeState(session));
            session.redoStack.clear();
            session.map = std::move(map);
            changedEntities->append(changedEntity(QStringLiteral("layer"), layerId, QStringLiteral("updated")));
            return true;
        }

        if (targetType == QStringLiteral("object")) {
            const int objectId = payload.value(QStringLiteral("targetId")).toInt();
            auto *object = map->findObjectById(objectId);
            if (!object) {
                if (errorMessage)
                    *errorMessage = QStringLiteral("Unknown object id: %1").arg(objectId);
                return false;
            }
            object->mergeProperties(properties);
            session.undoStack.append(serializeState(session));
            session.redoStack.clear();
            session.map = std::move(map);
            changedEntities->append(changedEntity(QStringLiteral("object"), objectId, QStringLiteral("updated")));
            return true;
        }

        if (errorMessage)
            *errorMessage = QStringLiteral("Unsupported targetType: %1").arg(targetType);
        return false;
    }

    if (errorMessage)
        *errorMessage = QStringLiteral("Unsupported command: %1").arg(type);
    return false;
}

CommandResult BridgeEngine::executeCommand(const QString &sessionId, const QJsonObject &command)
{
    auto *currentSession = session(sessionId);
    if (!currentSession)
        return commandFailure(nullptr, QStringLiteral("SESSION_NOT_FOUND"), QStringLiteral("Unknown session: %1").arg(sessionId));
    if (!hasLoadedDocument(*currentSession))
        return commandFailure(currentSession, QStringLiteral("DOCUMENT_NOT_OPEN"), QStringLiteral("No document is open for this session."));

    QString errorMessage;
    QJsonArray changedEntities;
    const int revisionBefore = currentSession->revision;
    if (!applyCommand(*currentSession, command, &errorMessage, &changedEntities))
        return commandFailure(currentSession, QStringLiteral("COMMAND_ERROR"), errorMessage);

    currentSession->revision += 1;
    CommandResult result;
    result.success = true;
    result.revisionBefore = revisionBefore;
    result.revisionAfter = currentSession->revision;
    result.changedEntities = changedEntities;
    result.undoDepth = currentSession->undoStack.size();
    result.redoDepth = currentSession->redoStack.size();
    return result;
}

ValidationResult BridgeEngine::validateDocument(const QString &sessionId) const
{
    const auto *currentSession = session(sessionId);
    if (!currentSession)
        return validationFailure(nullptr, QStringLiteral("SESSION_NOT_FOUND"), QStringLiteral("Unknown session: %1").arg(sessionId));
    if (!hasLoadedDocument(*currentSession))
        return validationFailure(currentSession, QStringLiteral("DOCUMENT_NOT_OPEN"), QStringLiteral("No document is open for this session."));

    ValidationResult result;
    result.success = true;
    result.revision = currentSession->revision;

    if (currentSession->kind == DocumentKind::Map) {
        QSet<int> layerIds;
        for (Tiled::Layer *layer : currentSession->map->layers()) {
            if (layerIds.contains(layer->id())) {
                result.diagnostics.append(QJsonObject{
                    { QStringLiteral("severity"), QStringLiteral("error") },
                    { QStringLiteral("code"), QStringLiteral("DUPLICATE_LAYER_ID") },
                    { QStringLiteral("message"), QStringLiteral("Duplicate layer id: %1").arg(layer->id()) },
                    { QStringLiteral("target"), QStringLiteral("layer:%1").arg(layer->id()) },
                });
            }
            layerIds.insert(layer->id());
        }

        QSet<int> objectIds;
        for (Tiled::Layer *layer : currentSession->map->layers()) {
            if (auto *group = dynamic_cast<Tiled::ObjectGroup*>(layer)) {
                for (Tiled::MapObject *object : group->objects()) {
                    if (objectIds.contains(object->id())) {
                        result.diagnostics.append(QJsonObject{
                            { QStringLiteral("severity"), QStringLiteral("error") },
                            { QStringLiteral("code"), QStringLiteral("DUPLICATE_OBJECT_ID") },
                            { QStringLiteral("message"), QStringLiteral("Duplicate object id: %1").arg(object->id()) },
                            { QStringLiteral("target"), QStringLiteral("object:%1").arg(object->id()) },
                        });
                    }
                    objectIds.insert(object->id());
                }
            }
        }

        const QDir baseDir = QFileInfo(currentSession->path).dir();
        for (const Tiled::SharedTileset &tileset : currentSession->map->tilesets()) {
            if (!tileset->fileName().isEmpty() && !QFileInfo::exists(tileset->fileName())) {
                result.diagnostics.append(QJsonObject{
                    { QStringLiteral("severity"), QStringLiteral("error") },
                    { QStringLiteral("code"), QStringLiteral("MISSING_TILESET") },
                    { QStringLiteral("message"), QStringLiteral("Missing tileset file: %1").arg(tileset->fileName()) },
                    { QStringLiteral("path"), tileset->fileName() },
                });
            }

            if (!tileset->imageSource().isEmpty()) {
                const QString imagePath = baseDir.absoluteFilePath(tileset->imageSource().toString());
                if (!QFileInfo::exists(imagePath)) {
                    result.diagnostics.append(QJsonObject{
                        { QStringLiteral("severity"), QStringLiteral("warning") },
                        { QStringLiteral("code"), QStringLiteral("MISSING_TILESET_IMAGE") },
                        { QStringLiteral("message"), QStringLiteral("Missing tileset image: %1").arg(imagePath) },
                        { QStringLiteral("path"), imagePath },
                    });
                }
            }
        }
    } else if (!currentSession->tileset->imageSource().isEmpty()) {
        const QDir baseDir = QFileInfo(currentSession->path).dir();
        const QString imagePath = baseDir.absoluteFilePath(currentSession->tileset->imageSource().toString());
        if (!QFileInfo::exists(imagePath)) {
            result.diagnostics.append(QJsonObject{
                { QStringLiteral("severity"), QStringLiteral("warning") },
                { QStringLiteral("code"), QStringLiteral("MISSING_TILESET_IMAGE") },
                { QStringLiteral("message"), QStringLiteral("Missing tileset image: %1").arg(imagePath) },
                { QStringLiteral("path"), imagePath },
            });
        }
    }

    return result;
}

SaveResult BridgeEngine::saveDocument(const QString &sessionId)
{
    auto *currentSession = session(sessionId);
    if (!currentSession)
        return saveFailure(nullptr, QStringLiteral("SESSION_NOT_FOUND"), QStringLiteral("Unknown session: %1").arg(sessionId));
    if (!hasLoadedDocument(*currentSession))
        return saveFailure(currentSession, QStringLiteral("DOCUMENT_NOT_OPEN"), QStringLiteral("No document is open for this session."));

    const auto validation = validateDocument(sessionId);
    if (!validation.success)
        return saveFailure(currentSession, validation.errorCode, validation.errorMessage);
    for (const auto diagnosticValue : validation.diagnostics) {
        if (diagnosticValue.toObject().value(QStringLiteral("severity")).toString() == QStringLiteral("error"))
            return saveFailure(currentSession, QStringLiteral("SAVE_BLOCKED_BY_VALIDATION"), QStringLiteral("Document validation failed."));
    }

    QString errorMessage;
    const QFileInfo fileInfo(currentSession->path);
    const QString suffix = fileInfo.suffix().toLower();

    bool saved = false;
    if (currentSession->kind == DocumentKind::Map && suffix == QStringLiteral("tmx")) {
        Tiled::MapWriter writer;
        saved = writer.writeMap(currentSession->map.get(), currentSession->path);
        if (!saved)
            errorMessage = writer.errorString();
    } else if (currentSession->kind == DocumentKind::Map && (suffix == QStringLiteral("tmj") || suffix == QStringLiteral("json"))) {
        const auto serialized = serializeState(*currentSession);
        saved = saveJsonVariant(serialized.variant, currentSession->path, &errorMessage);
    } else if (currentSession->kind == DocumentKind::Tileset && (suffix == QStringLiteral("tsj") || suffix == QStringLiteral("json"))) {
        const auto serialized = serializeState(*currentSession);
        saved = saveJsonVariant(serialized.variant, currentSession->path, &errorMessage);
    } else if (currentSession->kind == DocumentKind::Tileset && suffix == QStringLiteral("tsx")) {
        Tiled::MapWriter writer;
        saved = writer.writeTileset(*currentSession->tileset, currentSession->path);
        if (!saved)
            errorMessage = writer.errorString();
    } else {
        errorMessage = QStringLiteral("Unsupported save format for %1").arg(currentSession->path);
    }

    if (!saved)
        return saveFailure(currentSession, QStringLiteral("SAVE_ERROR"), errorMessage);

    SaveResult result;
    result.success = true;
    result.path = currentSession->path;
    result.revision = currentSession->revision;
    return result;
}

CommandResult BridgeEngine::undo(const QString &sessionId)
{
    auto *currentSession = session(sessionId);
    if (!currentSession)
        return commandFailure(nullptr, QStringLiteral("SESSION_NOT_FOUND"), QStringLiteral("Unknown session: %1").arg(sessionId));
    if (!hasLoadedDocument(*currentSession))
        return commandFailure(currentSession, QStringLiteral("DOCUMENT_NOT_OPEN"), QStringLiteral("No document is open for this session."));
    if (currentSession->undoStack.isEmpty())
        return commandFailure(currentSession, QStringLiteral("COMMAND_ERROR"), QStringLiteral("Undo stack is empty."));

    const auto previous = currentSession->undoStack.takeLast();
    currentSession->redoStack.append(serializeState(*currentSession));
    QString errorMessage;
    if (!restoreState(*currentSession, previous, &errorMessage))
        return commandFailure(currentSession, QStringLiteral("ENGINE_ERROR"), errorMessage);

    currentSession->revision = std::max(0, currentSession->revision - 1);

    CommandResult result;
    result.success = true;
    result.revisionBefore = currentSession->revision + 1;
    result.revisionAfter = currentSession->revision;
    result.undoDepth = currentSession->undoStack.size();
    result.redoDepth = currentSession->redoStack.size();
    return result;
}

CommandResult BridgeEngine::redo(const QString &sessionId)
{
    auto *currentSession = session(sessionId);
    if (!currentSession)
        return commandFailure(nullptr, QStringLiteral("SESSION_NOT_FOUND"), QStringLiteral("Unknown session: %1").arg(sessionId));
    if (!hasLoadedDocument(*currentSession))
        return commandFailure(currentSession, QStringLiteral("DOCUMENT_NOT_OPEN"), QStringLiteral("No document is open for this session."));
    if (currentSession->redoStack.isEmpty())
        return commandFailure(currentSession, QStringLiteral("COMMAND_ERROR"), QStringLiteral("Redo stack is empty."));

    const int revisionBefore = currentSession->revision;
    const auto next = currentSession->redoStack.takeLast();
    currentSession->undoStack.append(serializeState(*currentSession));
    QString errorMessage;
    if (!restoreState(*currentSession, next, &errorMessage))
        return commandFailure(currentSession, QStringLiteral("ENGINE_ERROR"), errorMessage);

    currentSession->revision += 1;

    CommandResult result;
    result.success = true;
    result.revisionBefore = revisionBefore;
    result.revisionAfter = currentSession->revision;
    result.undoDepth = currentSession->undoStack.size();
    result.redoDepth = currentSession->redoStack.size();
    return result;
}

bool BridgeEngine::closeSession(const QString &sessionId)
{
    return mSessions.remove(sessionId) > 0;
}

} // namespace tiledagent
