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

#include "editablegrouplayer.h"
#include "editableimagelayer.h"
#include "editablemap.h"
#include "editablemapobject.h"
#include "editableobjectgroup.h"
#include "editableselectedarea.h"
#include "editabletile.h"
#include "editabletilelayer.h"
#include "editabletileset.h"
#include "editablewangset.h"
#include "editableworld.h"
#include "logginginterface.h"
#include "mapeditor.h"
#include "mapview.h"
#include "preferences.h"
#include "project.h"
#include "projectmanager.h"
#include "regionvaluetype.h"
#include "scriptbase64.h"
#include "scriptdialog.h"
#include "scriptedaction.h"
#include "scriptedtool.h"
#include "scriptfile.h"
#include "scriptfileformatwrappers.h"
#include "scriptfileinfo.h"
#include "scriptgeometry.h"
#include "scriptimage.h"
#include "scriptmodule.h"
#include "scriptprocess.h"
#include "tilecollisiondock.h"
#include "tilelayer.h"
#include "tilelayeredit.h"
#include "tilelayerwangedit.h"
#include "tilesetdock.h"
#include "tileseteditor.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QQmlEngine>
#include <QStandardPaths>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QTextCodec>
#else
#include <QStringDecoder>
#endif
#include <QtDebug>

namespace Tiled {

static Preference<QStringList> scriptingEnabledProjects { "Scripting/EnabledProjects" };

ScriptManager *ScriptManager::mInstance;

ScriptManager &ScriptManager::instance()
{
    if (!mInstance)
        mInstance = new ScriptManager;
    return *mInstance;
}

void ScriptManager::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

/*
 * mEngine needs to be QQmlEngine for the "Qt" module to be available, which
 * is necessary to pass things like QSize or QPoint to some API functions
 * (using Qt.size and Qt.point).
 *
 * It also means we don't need to call QJSEngine::installExtensions, since the
 * QQmlEngine seems to include those by default.
 */

ScriptManager::ScriptManager(QObject *parent)
    : QObject(parent)
{
    mResetTimer.setInterval(500);
    mResetTimer.setSingleShot(true);
    connect(&mResetTimer, &QTimer::timeout, this, &ScriptManager::reset);

    qRegisterMetaType<AssetType::Value>("AssetType");
    qRegisterMetaType<Cell>();
    qRegisterMetaType<EditableAsset*>();
    qRegisterMetaType<EditableGroupLayer*>();
    qRegisterMetaType<EditableLayer*>();
    qRegisterMetaType<EditableMap*>();
    qRegisterMetaType<EditableMapObject*>();
    qRegisterMetaType<EditableObjectGroup*>();
    qRegisterMetaType<EditableSelectedArea*>();
    qRegisterMetaType<EditableTile*>();
    qRegisterMetaType<EditableTileLayer*>();
    qRegisterMetaType<EditableTileset*>();
    qRegisterMetaType<EditableWangSet*>();
    qRegisterMetaType<EditableWorld*>();
    qRegisterMetaType<Font>();
    qRegisterMetaType<MapEditor*>();
    qRegisterMetaType<MapView*>();
    qRegisterMetaType<RegionValueType>();
    qRegisterMetaType<QVector<Tiled::RegionValueType>>();
    qRegisterMetaType<ScriptedAction*>();
    qRegisterMetaType<ScriptedTool*>();
    qRegisterMetaType<TileCollisionDock*>();
    qRegisterMetaType<TileLayerEdit*>();
    qRegisterMetaType<TileLayerWangEdit*>();
    qRegisterMetaType<TilesetDock*>();
    qRegisterMetaType<TilesetEditor*>();
    qRegisterMetaType<ScriptMapFormatWrapper*>();
    qRegisterMetaType<ScriptTilesetFormatWrapper*>();
    qRegisterMetaType<ScriptImage*>();
    qRegisterMetaType<WangIndex::Value>("WangIndex");

    connect(&mWatcher, &FileSystemWatcher::pathsChanged,
            this, &ScriptManager::scriptFilesChanged);

    connect(ProjectManager::instance(), &ProjectManager::projectChanged,
            this, &ScriptManager::refreshExtensionsPaths);

    const QString configLocation { Preferences::instance()->configLocation() };
    if (!configLocation.isEmpty()) {
        mExtensionsPath = QDir{configLocation}.filePath(QStringLiteral("extensions"));

        if (!QFile::exists(mExtensionsPath))
            QDir().mkpath(mExtensionsPath);
    }
}

void ScriptManager::ensureInitialized()
{
    if (!mEngine) {
        if (mExtensionsPaths.isEmpty())
            refreshExtensionsPaths();

        initialize();
    }
}

void ScriptManager::setScriptArguments(const QStringList &arguments)
{
    Q_ASSERT(mModule);
    mModule->setScriptArguments(arguments);
}

QJSValue ScriptManager::evaluate(const QString &program,
                                 const QString &fileName, int lineNumber)
{
    QJSValue globalObject = mEngine->globalObject();
    if (!fileName.isEmpty())
        globalObject.setProperty(QStringLiteral("__filename"), fileName);

    QJSValue result = mEngine->evaluate(program, fileName, lineNumber);
    checkError(result, program);

    globalObject.deleteProperty(QStringLiteral("__filename"));
    return result;
}

void ScriptManager::evaluateFileOrLoadModule(const QString &fileName)
{
    if (fileName.endsWith(QLatin1String(".js"), Qt::CaseInsensitive)) {
        evaluateFile(fileName);
    } else {
        Tiled::INFO(tr("Importing module '%1'").arg(fileName));

        QJSValue globalObject = mEngine->globalObject();
        globalObject.setProperty(QStringLiteral("__filename"), fileName);

        QJSValue result = mEngine->importModule(fileName);

        // According to the documentation, importModule could return an
        // error object, though in practice this doesn't appear to happen.
        if (!checkError(result)) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
            // This appears to be the way to report exceptions
            checkError(mEngine->catchError());
#else
            // With Qt 5 it seems we need to get a little creative, like
            // calling evaluate to let that catch a potentially raised
            // exception.
            checkError(mEngine->evaluate(QString()));
#endif
        }

        globalObject.deleteProperty(QStringLiteral("__filename"));
    }
}

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
static bool fromUtf8(const QByteArray &bytes, QString &unicode)
{
    QTextCodec::ConverterState state;
    const QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    unicode = codec->toUnicode(bytes.constData(), bytes.size(), &state);
    return state.invalidChars == 0;
}
#endif

QJSValue ScriptManager::evaluateFile(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        Tiled::ERROR(tr("Error opening file: %1").arg(fileName));
        return QJSValue();
    }

    const QByteArray bytes = file.readAll();
    QString script;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    if (!fromUtf8(bytes, script))
        script = QTextCodec::codecForUtfText(bytes)->toUnicode(bytes);
#else
    auto encoding = QStringConverter::encodingForData(bytes.constData(), bytes.size());
    QStringDecoder decoder(encoding.value_or(QStringConverter::Utf8));
    script = decoder.decode(bytes);
    if (decoder.hasError()) {
        Tiled::ERROR(tr("Error decoding file: %1").arg(fileName));
        return QJSValue();
    }
#endif

    Tiled::INFO(tr("Evaluating '%1'").arg(fileName));
    return evaluate(script, fileName);
}

QString ScriptManager::createTempValue(const QJSValue &value)
{
    auto name = QLatin1Char('$') + QString::number(mTempCount++);
    mEngine->globalObject().setProperty(name, value);
    return name;
}

void ScriptManager::loadExtensions()
{
    QStringList extensionSearchPaths;

    for (const QString &extensionsPath : std::as_const(mExtensionsPaths)) {
        // Extension scripts and resources can also be in the top-level
        extensionSearchPaths.append(extensionsPath);

        // Each folder in an extensions path is expected to be an extension
        const QDir extensionsDir(extensionsPath);
        const QStringList dirs = extensionsDir.entryList(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
        for (const QString &dir : dirs)
            extensionSearchPaths.append(extensionsDir.filePath(dir));
    }

    QDir::setSearchPaths(QStringLiteral("ext"), extensionSearchPaths);

    for (const QString &extensionPath : std::as_const(extensionSearchPaths))
        loadExtension(extensionPath);
}

void ScriptManager::loadExtension(const QString &path)
{
    mWatcher.addPath(path);

    const QStringList nameFilters = {
        QLatin1String("*.js"),
        QLatin1String("*.mjs")
    };
    const QDir dir(path);
    const QStringList jsFiles = dir.entryList(nameFilters,
                                              QDir::Files | QDir::Readable);

    for (const QString &jsFile : jsFiles) {
        const QString absolutePath = dir.filePath(jsFile);
        evaluateFileOrLoadModule(absolutePath);
        mWatcher.addPath(absolutePath);
    }
}

bool ScriptManager::checkError(QJSValue value, const QString &program)
{
    if (!value.isError())
        return false;

    QString errorString = value.toString();
    QString stack = value.property(QStringLiteral("stack")).toString();

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    const auto stackEntries = QStringView(stack).split(QLatin1Char('\n'));
#else
    const auto stackEntries = stack.splitRef(QLatin1Char('\n'));
#endif
    if (stackEntries.size() > 0 && !stackEntries.first().startsWith(QLatin1String("%entry@"))) {
        // Add stack if there were more than one entries
        errorString.append(QLatin1Char('\n'));
        errorString.append(tr("Stack traceback:"));
        errorString.append(QLatin1Char('\n'));

        for (const auto &entry : stackEntries) {
            errorString.append(QStringLiteral("  "));
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

    mModule->error(errorString);
    return true;
}

void ScriptManager::throwError(const QString &message)
{
    engine()->throwError(message);
}

void ScriptManager::throwNullArgError(int argNumber)
{
    throwError(QCoreApplication::translate("Script Errors",
                                           "Argument %1 is undefined or the wrong type").arg(argNumber));
}

void ScriptManager::reset()
{
    // If resetting the script engine is currently blocked, which can happen
    // while a script is waiting for a popup, try again later.
    if (mResetBlocked) {
        mResetTimer.start();
        return;
    }

    Tiled::INFO(tr("Resetting script engine"));

    mWatcher.clear();

    delete mEngine;
    delete mModule;

    mEngine = nullptr;
    mModule = nullptr;
    mTempCount = 0;

    initialize();
}

void ScriptManager::initialize()
{
    auto engine = new QQmlEngine(this);

    // We'll report errors in the Console view instead
    engine->setOutputWarningsToStandardError(false);
    connect(engine, &QQmlEngine::warnings, this, &ScriptManager::onScriptWarnings);

    mEngine = engine;
    mModule = new ScriptModule(this);

    QJSValue globalObject = engine->globalObject();

    // Work around issue where since Qt 6, the value from the global Qt
    // namespace are no longer part of the Qt object.
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0) && QT_VERSION < QT_VERSION_CHECK(6,4,0)
    QJSValue qtObject = globalObject.property(QStringLiteral("Qt"));

    auto &qtNamespace = Qt::staticMetaObject;
    for (int i = qtNamespace.enumeratorCount(); i >= 0; --i) {
        auto metaEnum = qtNamespace.enumerator(i);
        for (int k = metaEnum.keyCount(); k >= 0; --k)
            qtObject.setProperty(QString::fromLatin1(metaEnum.key(k)), metaEnum.value(k));
    }
#endif

    globalObject.setProperty(QStringLiteral("tiled"), engine->newQObject(mModule));
    globalObject.setProperty(QStringLiteral("Tiled"), engine->newQMetaObject<ScriptModule>());
    globalObject.setProperty(QStringLiteral("AssetType"), engine->newQMetaObject(&AssetType::staticMetaObject));
    globalObject.setProperty(QStringLiteral("GroupLayer"), engine->newQMetaObject<EditableGroupLayer>());
    globalObject.setProperty(QStringLiteral("Image"), engine->newQMetaObject<ScriptImage>());
    globalObject.setProperty(QStringLiteral("ImageLayer"), engine->newQMetaObject<EditableImageLayer>());
    globalObject.setProperty(QStringLiteral("Layer"), engine->newQMetaObject<EditableLayer>());
    globalObject.setProperty(QStringLiteral("MapObject"), engine->newQMetaObject<EditableMapObject>());
    globalObject.setProperty(QStringLiteral("ObjectGroup"), engine->newQMetaObject<EditableObjectGroup>());
    globalObject.setProperty(QStringLiteral("Tile"), engine->newQMetaObject<EditableTile>());
    globalObject.setProperty(QStringLiteral("TileLayer"), engine->newQMetaObject<EditableTileLayer>());
    globalObject.setProperty(QStringLiteral("TileMap"), engine->newQMetaObject<EditableMap>());
    globalObject.setProperty(QStringLiteral("Tileset"), engine->newQMetaObject<EditableTileset>());
    globalObject.setProperty(QStringLiteral("WangIndex"), engine->newQMetaObject(&WangIndex::staticMetaObject));
    globalObject.setProperty(QStringLiteral("WangSet"), engine->newQMetaObject<EditableWangSet>());

    registerBase64(engine);
    registerDialog(engine);
    registerFile(engine);
    registerFileInfo(engine);
    registerGeometry(engine);
    registerProcess(engine);
    loadExtensions();
}

void ScriptManager::onScriptWarnings(const QList<QQmlError> &warnings)
{
    for (const auto &warning : warnings) {
        Tiled::ERROR(warning.toString(), [url = warning.url()] {
            if (!url.isEmpty())
                QDesktopServices::openUrl(url);
        });
    }
}

void ScriptManager::scriptFilesChanged(const QStringList &scriptFiles)
{
    Tiled::INFO(tr("Script files changed: %1").arg(scriptFiles.join(QLatin1String(", "))));
    reset();
}

void ScriptManager::refreshExtensionsPaths()
{
    QStringList extensionsPaths;

    if (!mExtensionsPath.isEmpty())
        extensionsPaths.append(mExtensionsPath);

    // Add extensions path from project
    bool projectExtensionsSuppressed = false;
    const Project &project = ProjectManager::instance()->project();
    if (!project.mExtensionsPath.isEmpty()) {
        const QFileInfo info(project.mExtensionsPath);
        if (info.exists() && info.isDir()) {
            if (scriptingEnabledProjects.get().contains(project.fileName(), Qt::CaseInsensitive))
                extensionsPaths.append(project.mExtensionsPath);
            else
                projectExtensionsSuppressed = true;
        }
    }

    if (mProjectExtensionsSuppressed != projectExtensionsSuppressed) {
        mProjectExtensionsSuppressed = projectExtensionsSuppressed;
        emit projectExtensionsSuppressedChanged(projectExtensionsSuppressed);
    }

    extensionsPaths.sort();
    extensionsPaths.removeDuplicates();

    if (extensionsPaths == mExtensionsPaths)
        return;

    mExtensionsPaths.swap(extensionsPaths);

    if (mEngine) {
        Tiled::INFO(tr("Extensions paths changed: %1").arg(mExtensionsPaths.join(QLatin1String(", "))));
        reset();
    }
}

void ScriptManager::enableProjectExtensions()
{
    const Project &project = ProjectManager::instance()->project();
    const QString &fileName = project.fileName();
    if (!fileName.isEmpty()) {
        QStringList projects = scriptingEnabledProjects;
        if (!projects.contains(fileName, Qt::CaseInsensitive)) {
            projects.append(fileName);
            scriptingEnabledProjects = projects;

            refreshExtensionsPaths();
        }
    }
}

} // namespace Tiled

#include "moc_scriptmanager.cpp"
