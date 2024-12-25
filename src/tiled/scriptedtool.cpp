/*
 * scriptedtool.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scriptedtool.h"

#include "actionmanager.h"
#include "brushitem.h"
#include "editablemap.h"
#include "editabletile.h"
#include "logginginterface.h"
#include "mapdocument.h"
#include "pluginmanager.h"
#include "scriptmanager.h"
#include "tile.h"
#include "tilesetdocument.h"

#include <QCoreApplication>
#include <QJSEngine>
#include <QKeyEvent>
#include <QQmlEngine>
#include <QToolBar>

namespace Tiled {

ScriptedTool::ScriptedTool(Id id, QJSValue object, QObject *parent)
    : AbstractTileTool(id, QStringLiteral("<unnamed tool>"), QIcon(), QKeySequence(), nullptr, parent)
    , mScriptObject(std::move(object))
{
    // Read out the properties from the script object before setting its prototype
    const QJSValue nameProperty = mScriptObject.property(QStringLiteral("name"));
    const QJSValue iconProperty = mScriptObject.property(QStringLiteral("icon"));
    const QJSValue toolBarActionsProperty = mScriptObject.property(QStringLiteral("toolBarActions"));
    const QJSValue usesSelectedTilesProperty = mScriptObject.property(QStringLiteral("usesSelectedTiles"));
    const QJSValue usesWangSetsProperty = mScriptObject.property(QStringLiteral("usesWangSets"));
    const QJSValue targetLayerTypeProperty = mScriptObject.property(QStringLiteral("targetLayerType"));

    // Make members of ScriptedTool available through the original object
    auto &scriptManager = ScriptManager::instance();
    auto self = scriptManager.engine()->newQObject(this);
    mScriptObject.setPrototype(self);

    if (nameProperty.isString())
        setName(nameProperty.toString());

    if (iconProperty.isString())
        setIconFileName(iconProperty.toString());

    if (toolBarActionsProperty.isArray()) {
        QStringList actionNames;
        const int length = toolBarActionsProperty.property(QStringLiteral("length")).toInt();
        for (int i = 0; i < length; ++i)
            actionNames.append(toolBarActionsProperty.property(i).toString());
        setToolBarActions(actionNames);
    }

    if (usesSelectedTilesProperty.isBool())
        setUsesSelectedTiles(usesSelectedTilesProperty.toBool());

    if (usesWangSetsProperty.isBool())
        setUsesWangSets(usesWangSetsProperty.toBool());

    if (targetLayerTypeProperty.isNumber())
        setTargetLayerType(targetLayerTypeProperty.toInt());
    else
        setTargetLayerType(0);  // default behavior is not to disable based on current layer

    PluginManager::addObject(this);
}

ScriptedTool::~ScriptedTool()
{
    PluginManager::removeObject(this);
}

EditableMap *ScriptedTool::editableMap() const
{
    return mapDocument() ? static_cast<EditableMap*>(mapDocument()->editable())
                         : nullptr;
}

EditableTile *ScriptedTool::editableTile() const
{
    if (Tile *t = tile()) {
        auto tileset = t->tileset()->sharedFromThis();

        if (auto tilesetDocument = TilesetDocument::findDocumentForTileset(tileset)) {
            EditableTileset *editable = tilesetDocument->editable();
            return editable->tile(t->id());
        }
    }

    return nullptr;
}

EditableMap *ScriptedTool::preview() const
{
    const auto &map = brushItem()->map();
    if (!map)
        return nullptr;

    auto editableMap = new EditableMap(map->clone());
    QQmlEngine::setObjectOwnership(editableMap, QQmlEngine::JavaScriptOwnership);
    return editableMap;
}

void ScriptedTool::setPreview(EditableMap *editableMap)
{
    if (!editableMap) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }
    // todo: filter any non-tilelayers out of the map?
    auto map = editableMap->map()->clone();
    brushItem()->setMap(SharedMap { map.release() });
}

void ScriptedTool::activate(MapScene *scene)
{
    AbstractTileTool::activate(scene);
    call(QStringLiteral("activated"));
}

void ScriptedTool::deactivate(MapScene *scene)
{
    AbstractTileTool::deactivate(scene);
    call(QStringLiteral("deactivated"));
}

void ScriptedTool::keyPressed(QKeyEvent *keyEvent)
{
    QJSValueList args;
    args.append(keyEvent->key());
    args.append(static_cast<int>(keyEvent->modifiers()));

    call(QStringLiteral("keyPressed"), args);
}

void ScriptedTool::mouseEntered()
{
    AbstractTileTool::mouseEntered();
    call(QStringLiteral("mouseEntered"));
}

void ScriptedTool::mouseLeft()
{
    AbstractTileTool::mouseLeft();
    call(QStringLiteral("mouseLeft"));
}

void ScriptedTool::mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers)
{
    AbstractTileTool::mouseMoved(pos, modifiers);

    QJSValueList args;
    args.append(pos.x());
    args.append(pos.y());
    args.append(static_cast<int>(modifiers));

    call(QStringLiteral("mouseMoved"), args);
}

void ScriptedTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    AbstractTileTool::mousePressed(event);
    if (event->isAccepted())
        return;

    QJSValueList args;
    args.append(event->button());
    args.append(event->pos().x());
    args.append(event->pos().y());
    args.append(static_cast<int>(event->modifiers()));

    call(QStringLiteral("mousePressed"), args);
    event->accept();
}

void ScriptedTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    QJSValueList args;
    args.append(event->button());
    args.append(event->pos().x());
    args.append(event->pos().y());
    args.append(static_cast<int>(event->modifiers()));

    call(QStringLiteral("mouseReleased"), args);
}

void ScriptedTool::mouseDoubleClicked(QGraphicsSceneMouseEvent *event)
{
    QJSValueList args;
    args.append(event->button());
    args.append(event->pos().x());
    args.append(event->pos().y());
    args.append(static_cast<int>(event->modifiers()));

    if (!call(QStringLiteral("mouseDoubleClicked"), args))
        mousePressed(event);
}

void ScriptedTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    QJSValueList args;
    args.append(static_cast<int>(modifiers));

    call(QStringLiteral("modifiersChanged"), args);
}

void ScriptedTool::languageChanged()
{
    call(QStringLiteral("languageChanged"));
}

void ScriptedTool::populateToolBar(QToolBar *toolBar)
{
    for (const Id actionId : mToolBarActions) {
        if (actionId == "-")
            toolBar->addSeparator();
        else if (auto action = ActionManager::findAction(actionId))
            toolBar->addAction(action);
        else
            Tiled::ERROR(QCoreApplication::translate("Script Errors", "Could not find action '%1'").arg(actionId.toString()));
    }
}

bool ScriptedTool::validateToolObject(QJSValue value)
{
    const QJSValue nameProperty = value.property(QStringLiteral("name"));

    if (!nameProperty.isString()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid tool object (requires string 'name' property)"));
        return false;
    }

    return true;
}

void ScriptedTool::setIconFileName(const QString &fileName)
{
    if (mIconFileName == fileName)
        return;

    mIconFileName = fileName;

    QString iconFile = fileName;

    const QString ext = QStringLiteral("ext:");
    if (!iconFile.startsWith(ext) && !iconFile.startsWith(QLatin1Char(':')))
        iconFile.prepend(ext);

    setIcon(QIcon { iconFile });
}

QStringList ScriptedTool::toolBarActions() const
{
    return idsToNames(mToolBarActions);
}

void ScriptedTool::setToolBarActions(const QStringList &actionNames)
{
    mToolBarActions = namesToIds(actionNames);
}

void ScriptedTool::mapDocumentChanged(MapDocument *oldDocument,
                                      MapDocument *newDocument)
{
    AbstractTileTool::mapDocumentChanged(oldDocument, newDocument);

    auto engine = ScriptManager::instance().engine();

    QJSValueList args;
    args.append(oldDocument ? engine->newQObject(oldDocument->editable()) : QJSValue::NullValue);
    args.append(newDocument ? engine->newQObject(newDocument->editable()) : QJSValue::NullValue);

    call(QStringLiteral("mapChanged"), args);
}

void ScriptedTool::tilePositionChanged(QPoint tilePos)
{
    QJSValueList args;
    args.append(tilePos.x());
    args.append(tilePos.y());

    call(QStringLiteral("tilePositionChanged"), args);
}

void ScriptedTool::updateStatusInfo()
{
    if (!call(QStringLiteral("updateStatusInfo")))
        AbstractTileTool::updateStatusInfo();
}

void ScriptedTool::updateEnabledState()
{
    if (!call(QStringLiteral("updateEnabledState"))) {
        AbstractTileTool::updateEnabledState();
        return;
    }

    updateBrushVisibility();
}

bool ScriptedTool::call(const QString &methodName, const QJSValueList &args)
{
    QJSValue method = mScriptObject.property(methodName);
    if (method.isCallable()) {
        auto &scriptManager = ScriptManager::instance();
        QJSValue result = method.callWithInstance(mScriptObject, args);
        scriptManager.checkError(result);

        return true;
    }
    return false;
}

} // namespace Tiled

#include "moc_scriptedtool.cpp"
