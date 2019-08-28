/*
 * scriptmanager.cpp
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "scriptmanager.h"

#include "documentmanager.h"
#include "editablemap.h"
#include "editablemapobject.h"
#include "editableobjectgroup.h"
#include "editableselectedarea.h"
#include "editableterrain.h"
#include "editabletile.h"
#include "editabletilelayer.h"
#include "editabletileset.h"
#include "logginginterface.h"
#include "mapeditor.h"
#include "regionvaluetype.h"
#include "scriptedaction.h"
#include "scriptedtool.h"
#include "scriptmodule.h"
#include "tilecollisiondock.h"
#include "tilelayer.h"
#include "tilelayeredit.h"
#include "tilesetdock.h"
#include "tileseteditor.h"

#include <QDir>
#include <QFile>
#include <QQmlEngine>
#include <QStandardPaths>
#include <QTextCodec>
#include <QtDebug>

namespace Tiled {

std::unique_ptr<ScriptManager> ScriptManager::mInstance;

ScriptManager &ScriptManager::instance()
{
    if (!mInstance)
        mInstance.reset(new ScriptManager);
    return *mInstance;
}

void ScriptManager::deleteInstance()
{
    mInstance.reset();
}

/*
 * mJSEngine needs to be QQmlEngine for the "Qt" module to be available, which
 * is necessary to pass things like QSize or QPoint to some API functions
 * (using Qt.size and Qt.point).
 *
 * It also means we don't need to call QJSEngine::installExtensions, since the
 * QQmlEngine seems to include those by default.
 */

ScriptManager::ScriptManager(QObject *parent)
    : QObject(parent)
    , mEngine(new QQmlEngine(this))
    , mModule(new ScriptModule(this))
{
    qRegisterMetaType<Cell>();
    qRegisterMetaType<EditableAsset*>();
    qRegisterMetaType<EditableLayer*>();
    qRegisterMetaType<EditableMap*>();
    qRegisterMetaType<EditableMapObject*>();
    qRegisterMetaType<EditableObjectGroup*>();
    qRegisterMetaType<EditableSelectedArea*>();
    qRegisterMetaType<EditableTerrain*>();
    qRegisterMetaType<EditableTile*>();
    qRegisterMetaType<EditableTileLayer*>();
    qRegisterMetaType<EditableTileset*>();
    qRegisterMetaType<Font>();
    qRegisterMetaType<MapEditor*>();
    qRegisterMetaType<RegionValueType>();
    qRegisterMetaType<ScriptedAction*>();
    qRegisterMetaType<ScriptedTool*>();
    qRegisterMetaType<TileCollisionDock*>();
    qRegisterMetaType<TileLayerEdit*>();
    qRegisterMetaType<TilesetEditor*>();
    qRegisterMetaType<TilesetDock*>();

    connect(&mWatcher, &FileSystemWatcher::filesChanged,
            this, &ScriptManager::scriptFilesChanged);

    const QString configLocation { QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) };
    if (!configLocation.isEmpty()) {
        mExtensionsPath = QDir{configLocation}.filePath(QStringLiteral("extensions"));

        if (!QFile::exists(mExtensionsPath))
            QDir().mkpath(mExtensionsPath);

        mExtensionsPaths.append(mExtensionsPath);
    }
}

void ScriptManager::initialize()
{
    QJSValue globalObject = mEngine->globalObject();
    globalObject.setProperty(QStringLiteral("tiled"), mEngine->newQObject(mModule));
#if QT_VERSION >= 0x050800
    globalObject.setProperty(QStringLiteral("Layer"), mEngine->newQMetaObject<EditableLayer>());
    globalObject.setProperty(QStringLiteral("MapObject"), mEngine->newQMetaObject<EditableMapObject>());
    globalObject.setProperty(QStringLiteral("ObjectGroup"), mEngine->newQMetaObject<EditableObjectGroup>());
    globalObject.setProperty(QStringLiteral("Terrain"), mEngine->newQMetaObject<EditableTerrain>());
    globalObject.setProperty(QStringLiteral("Tile"), mEngine->newQMetaObject<EditableTile>());
    globalObject.setProperty(QStringLiteral("TileLayer"), mEngine->newQMetaObject<EditableTileLayer>());
    globalObject.setProperty(QStringLiteral("TileMap"), mEngine->newQMetaObject<EditableMap>());
    globalObject.setProperty(QStringLiteral("Tileset"), mEngine->newQMetaObject<EditableTileset>());
#endif

    evaluateStartupScripts();
    loadExtensions();
}

QJSValue ScriptManager::evaluate(const QString &program,
                                 const QString &fileName, int lineNumber)
{
    QJSValue result = mEngine->evaluate(program, fileName, lineNumber);
    checkError(result, program);
    return result;
}

QJSValue ScriptManager::evaluateFile(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        Tiled::ERROR(tr("Error opening file: %1").arg(fileName));
        return QJSValue();
    }

    const QByteArray text = file.readAll();
    const QString script = QTextCodec::codecForUtfText(text)->toUnicode(text);

    Tiled::INFO(tr("Evaluating '%1'").arg(fileName));
    return evaluate(script, fileName);
}

void ScriptManager::evaluateStartupScripts()
{
    const QStringList configLocations = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    for (const QString &configLocation : configLocations) {
        const QString scriptFile = configLocation + QLatin1String("/startup.js");
        if (QFile::exists(scriptFile)) {
            evaluateFile(scriptFile);
            mWatcher.addPath(scriptFile);
        }
    }
}

void ScriptManager::loadExtensions()
{
    QStringList extensionSearchPaths;

    // Each folder in an extensions path is expected to be an extension
    for (const QString &extensionsPath : qAsConst(mExtensionsPaths)) {
        const QDir extensionsDir(extensionsPath);
        const QStringList dirs = extensionsDir.entryList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
        for (const QString &dir : dirs)
            extensionSearchPaths.append(extensionsDir.filePath(dir));
    }

    QDir::setSearchPaths(QStringLiteral("ext"), extensionSearchPaths);

    for (const QString &extensionPath : extensionSearchPaths)
        loadExtension(extensionPath);
}

void ScriptManager::loadExtension(const QString &path)
{
    const QDir dir(path);
    const QStringList jsFiles = dir.entryList({ QLatin1String("*.js") },
                                              QDir::Files | QDir::Readable);

    for (const QString &jsFile : jsFiles) {
        const QString absolutePath = dir.filePath(jsFile);
        evaluateFile(absolutePath);
        mWatcher.addPath(absolutePath);
    }
}

void ScriptManager::checkError(QJSValue value, const QString &program)
{
    if (!value.isError())
        return;

    QString errorString = value.toString();
    QString stack = value.property(QStringLiteral("stack")).toString();

    const auto stackEntries = stack.splitRef(QLatin1Char('\n'));
    if (stackEntries.size() > 0 && !stackEntries.first().startsWith(QLatin1String("%entry@"))) {
        // Add stack if there were more than one entries
        errorString.append(QLatin1Char('\n'));
        errorString.append(tr("Stack traceback:"));
        errorString.append(QLatin1Char('\n'));

        for (const auto &entry : stackEntries) {
            errorString.append(QLatin1String("  "));
            errorString.append(entry);
            errorString.append(QLatin1Char('\n'));
        }

        errorString.chop(1);
    } else if (program.isEmpty() || program.contains(QLatin1Char('\n'))) {
        // Add line number when script spanned multiple lines
        errorString = tr("At line %1: %2")
                .arg(value.property(QStringLiteral("lineNumber")).toInt())
                .arg(errorString);
    }

    emit mModule->error(errorString);
}

void ScriptManager::throwError(const QString &message)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
    module()->error(message);
#else
    engine()->throwError(message);
#endif
}

void ScriptManager::reset()
{
    Tiled::INFO(tr("Resetting script engine"));

    mWatcher.clear();
    delete mEngine;
    delete mModule;

    mEngine = new QQmlEngine(this);
    mModule = new ScriptModule(this);

    initialize();
}

void ScriptManager::scriptFilesChanged(const QStringList &scriptFiles)
{
    Tiled::INFO(tr("Script files changed: %1").arg(scriptFiles.join(QLatin1String(", "))));
    reset();
}

} // namespace Tiled
