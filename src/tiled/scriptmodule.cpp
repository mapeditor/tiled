/*
 * scriptmodule.cpp
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

#include "scriptmodule.h"

#include "actionmanager.h"
#include "commandmanager.h"
#include "compression.h"
#include "documentmanager.h"
#include "editabletileset.h"
#include "issuesmodel.h"
#include "logginginterface.h"
#include "mainwindow.h"
#include "mapeditor.h"
#include "projectmanager.h"
#include "scriptdialog.h"
#include "scriptedaction.h"
#include "scriptedfileformat.h"
#include "scriptedtool.h"
#include "scriptfileformatwrappers.h"
#include "scriptmanager.h"
#include "tilesetdocument.h"
#include "tileseteditor.h"
#include "world.h"
#include "worlddocument.h"
#include "worldmanager.h"

#include <QAction>
#include <QCoreApplication>
#include <QFileDialog> 
#include <QInputDialog>
#include <QLibraryInfo>
#include <QMenu>
#include <QMessageBox>
#include <QQmlEngine>
#include <QVersionNumber>

namespace Tiled {

ScriptModule::ScriptModule(QObject *parent)
    : QObject(parent)
{
    // If the script module is only created for command-line use, there will
    // not be a DocumentManager instance.
    if (auto documentManager = DocumentManager::maybeInstance()) {
        connect(documentManager, &DocumentManager::documentCreated, this, &ScriptModule::documentCreated);
        connect(documentManager, &DocumentManager::documentOpened, this, &ScriptModule::documentOpened);
        connect(documentManager, &DocumentManager::documentAboutToBeSaved, this, &ScriptModule::documentAboutToBeSaved);
        connect(documentManager, &DocumentManager::documentSaved, this, &ScriptModule::documentSaved);
        connect(documentManager, &DocumentManager::documentAboutToClose, this, &ScriptModule::documentAboutToClose);
        connect(documentManager, &DocumentManager::currentDocumentChanged, this, &ScriptModule::currentDocumentChanged);

        connect(&WorldManager::instance(), &WorldManager::worldsChanged, this, &ScriptModule::worldsChanged);
    }
}

ScriptModule::~ScriptModule()
{
    for (const auto &pair : mRegisteredActions)
        ActionManager::unregisterAction(pair.second.get(), pair.first);

    ActionManager::clearMenuExtensions();

    IssuesModel::instance().removeIssuesWithContext(this);
    ScriptDialog::deleteAllDialogs();
}

QString ScriptModule::version() const
{
    return QCoreApplication::applicationVersion();
}

QString ScriptModule::qtVersion() const
{
    return QString::fromLatin1(qVersion());
}

QString ScriptModule::platform() const
{
#if defined(Q_OS_WIN)
    return QStringLiteral("windows");
#elif defined(Q_OS_MAC)
    return QStringLiteral("macos");
#elif defined(Q_OS_LINUX)
    return QStringLiteral("linux");
#else
    return QStringLiteral("unix");
#endif
}

QString ScriptModule::arch() const
{
#if defined(Q_PROCESSOR_X86_64)
    return QStringLiteral("x64");
#elif defined(Q_PROCESSOR_X86)
    return QStringLiteral("x86");
#else
    return QStringLiteral("unknown");
#endif
}

QString ScriptModule::extensionsPath() const
{
    return ScriptManager::instance().extensionsPath();
}

QString ScriptModule::applicationDirPath() const
{
    return QCoreApplication::applicationDirPath();
}

QString ScriptModule::projectFilePath() const
{
    return ProjectManager::instance()->project().fileName();
}

QStringList ScriptModule::scriptArguments() const
{
    return mScriptArguments;
}

void ScriptModule::setScriptArguments(const QStringList &arguments)
{
    mScriptArguments = arguments;
}

QStringList ScriptModule::actions() const
{
    return idsToNames(ActionManager::actions());
}

QStringList ScriptModule::menus() const
{
    return idsToNames(ActionManager::menus());
}

QStringList ScriptModule::mapFormats() const
{
    const auto formats = PluginManager::objects<MapFormat>();
    QStringList ret;
    ret.reserve(formats.length());
    for (auto format : formats)
        ret.append(format->shortName());

    return ret;
}

QStringList ScriptModule::tilesetFormats() const
{
    const auto formats = PluginManager::objects<TilesetFormat>();
    QStringList ret;
    ret.reserve(formats.length());
    for (auto format : formats)
        ret.append(format->shortName());

    return ret;
}

EditableAsset *ScriptModule::activeAsset() const
{
    if (auto documentManager = DocumentManager::maybeInstance())
        if (Document *document = documentManager->currentDocument())
            return document->editable();

    return nullptr;
}

bool ScriptModule::setActiveAsset(EditableAsset *asset) const
{
    if (!asset) {
        ScriptManager::instance().throwNullArgError(0);
        return false;
    }

    auto documentManager = DocumentManager::maybeInstance();
    if (!documentManager) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Editor not available"));
        return false;
    }

    if (asset->checkReadOnly())
        return false;

    if (auto document = asset->document())
        return documentManager->switchToDocument(document);

    if (auto document = asset->createDocument()) {
        documentManager->addDocument(document);
        return true;
    }

    return false;
}

QList<QObject *> ScriptModule::openAssets() const
{
    QList<QObject *> assets;
    if (auto documentManager = DocumentManager::maybeInstance()) {
        assets.reserve(documentManager->documents().size());
        for (const DocumentPtr &document : documentManager->documents())
            assets.append(document->editable());
    }
    return assets;
}

EditableAsset *ScriptModule::project()
{
    return ProjectManager::instance()->editableProject();
}

TilesetEditor *ScriptModule::tilesetEditor() const
{
    if (auto documentManager = DocumentManager::maybeInstance())
        return static_cast<TilesetEditor*>(documentManager->editor(Document::TilesetDocumentType));
    return nullptr;
}

MapEditor *ScriptModule::mapEditor() const
{
    if (auto documentManager = DocumentManager::maybeInstance())
        return static_cast<MapEditor*>(documentManager->editor(Document::MapDocumentType));
    return nullptr;
}

FilePath ScriptModule::filePath(const QUrl &path) const
{
    return { path };
}

ObjectRef ScriptModule::objectRef(int id) const
{
    return { id };
}

QVariant ScriptModule::propertyValue(const QString &typeName, const QVariant &value) const
{
    auto type = Object::propertyTypes().findPropertyValueType(typeName);
    if (!type) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Unknown type: %1").arg(typeName));
        return {};
    }

    switch (type->type) {
    case PropertyType::PT_Invalid:
    case PropertyType::PT_Class:
        break;
    case PropertyType::PT_Enum:
        // Call toPropertyValue to support using strings to create a value
        return type->toPropertyValue(value, ExportContext());
    }

    return type->wrap(value);
}

bool ScriptModule::versionLessThan(const QString &a, const QString &b)
{
    return QVersionNumber::fromString(a) < QVersionNumber::fromString(b);
}

EditableAsset *ScriptModule::open(const QString &fileName) const
{
    auto documentManager = DocumentManager::maybeInstance();
    if (!documentManager) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Editor not available"));
        return nullptr;
    }

    documentManager->openFile(fileName);

    // If opening succeeded, it is the current document
    int index = documentManager->findDocument(fileName);
    if (index != -1)
        if (auto document = documentManager->currentDocument())
            return document->editable();

    return nullptr;
}

bool ScriptModule::close(EditableAsset *asset) const
{
    if (!asset) {
        ScriptManager::instance().throwNullArgError(0);
        return false;
    }

    auto documentManager = DocumentManager::maybeInstance();
    int index = -1;

    if (documentManager)
        index = documentManager->findDocument(asset->document());

    if (index == -1) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Not an open asset"));
        return false;
    }

    documentManager->closeDocumentAt(index);
    return true;
}

EditableAsset *ScriptModule::reload(EditableAsset *asset) const
{
    if (!asset) {
        ScriptManager::instance().throwNullArgError(0);
        return nullptr;
    }

    auto documentManager = DocumentManager::maybeInstance();
    int index = -1;

    if (documentManager)
        index = documentManager->findDocument(asset->document());

    if (index == -1) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Not an open asset"));
        return nullptr;
    }

    if (auto editableTileset = qobject_cast<EditableTileset*>(asset)) {
        if (editableTileset->tilesetDocument()->isEmbedded()) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Can't reload an embedded tileset"));
            return nullptr;
        }
    }

    // The reload is going to invalidate the EditableAsset instance and
    // possibly also its document. We'll try to find it by its file name.
    const auto fileName = asset->fileName();

    if (documentManager->reloadDocumentAt(index)) {
        int newIndex = documentManager->findDocument(fileName);
        if (newIndex != -1)
            return documentManager->documents().at(newIndex)->editable();
    }

    return nullptr;
}

ScriptedAction *ScriptModule::registerAction(const QByteArray &idName, QJSValue callback)
{
    if (idName.isEmpty()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid ID"));
        return nullptr;
    }

    if (!callback.isCallable()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid callback function"));
        return nullptr;
    }

    Id id { idName };
    auto &action = mRegisteredActions[id];

    // Remove any previously registered action with the same name
    if (action) {
        ActionManager::unregisterAction(action.get(), id);
    } else if (ActionManager::findAction(id)) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Reserved ID"));
        return nullptr;
    }

    action = std::make_unique<ScriptedAction>(id, callback, this);
    ActionManager::registerAction(action.get(), id);
    return action.get();
}

void ScriptModule::registerMapFormat(const QString &shortName, QJSValue mapFormatObject)
{
    if (shortName.isEmpty()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid shortName"));
        return;
    }

    if (!ScriptedFileFormat::validateFileFormatObject(mapFormatObject))
        return;

    auto &format = mRegisteredMapFormats[shortName];
    format = std::make_unique<ScriptedMapFormat>(shortName, mapFormatObject, this);
}

void ScriptModule::registerTilesetFormat(const QString &shortName, QJSValue tilesetFormatObject)
{
    if (shortName.isEmpty()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid shortName"));
        return;
    }

    if (!ScriptedFileFormat::validateFileFormatObject(tilesetFormatObject))
        return;

    auto &format = mRegisteredTilesetFormats[shortName];
    format = std::make_unique<ScriptedTilesetFormat>(shortName, tilesetFormatObject, this);
}

QJSValue ScriptModule::registerTool(const QString &shortName, QJSValue toolObject)
{
    if (shortName.isEmpty()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid shortName"));
        return QJSValue();
    }

    if (!ScriptedTool::validateToolObject(toolObject))
        return QJSValue();

    Id id { shortName.toUtf8() };
    auto &tool = mRegisteredTools[id];

    tool = std::make_unique<ScriptedTool>(id, toolObject, this);
    return toolObject;
}

ScriptMapFormatWrapper *ScriptModule::mapFormat(const QString &shortName) const
{
    const auto formats = PluginManager::objects<MapFormat>();
    for (auto format : formats) {
        if (format->shortName() == shortName)
            return new ScriptMapFormatWrapper(format);
    }

    return nullptr;
}

ScriptMapFormatWrapper *ScriptModule::mapFormatForFile(const QString &fileName) const
{
    const auto formats = PluginManager::objects<MapFormat>();
    for (auto format : formats) {
        if (format->supportsFile(fileName))
            return new ScriptMapFormatWrapper(format);
    }

    return nullptr;
}

ScriptTilesetFormatWrapper *ScriptModule::tilesetFormat(const QString &shortName) const
{
    const auto formats = PluginManager::objects<TilesetFormat>();
    for (auto format : formats) {
        if (format->shortName() == shortName)
            return new ScriptTilesetFormatWrapper(format);
    }

    return nullptr;
}

ScriptTilesetFormatWrapper *ScriptModule::tilesetFormatForFile(const QString &fileName) const
{
    const auto formats = PluginManager::objects<TilesetFormat>();
    for (auto format : formats) {
        if (format->supportsFile(fileName))
            return new ScriptTilesetFormatWrapper(format);
    }

    return nullptr;
}


static QString toString(QJSValue value)
{
    if (value.isString())
        return value.toString();
    return QString();
}

static Id toId(QJSValue value)
{
    return Id(toString(value).toUtf8());
}

void ScriptModule::extendMenu(const QByteArray &idName, QJSValue items)
{
    ActionManager::MenuExtension extension;
    Id menuId(idName);

    if (!ActionManager::hasMenu(menuId)) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Unknown menu"));
        return;
    }

    auto addItem = [&] (QJSValue item) -> bool {
        ActionManager::MenuItem menuItem;

        const QJSValue action = item.property(QStringLiteral("action"));

        menuItem.action = toId(action);
        menuItem.beforeAction = toId(item.property(QStringLiteral("before")));
        menuItem.isSeparator = item.property(QStringLiteral("separator")).toBool();

        if (!menuItem.action.isNull()) {
            if (menuItem.isSeparator) {
                ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Separators can't have actions"));
                return false;
            }

            if (!ActionManager::findAction(menuItem.action)) {
                ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Unknown action: '%1'").arg(
                                                         QString::fromUtf8(menuItem.action.name())));
                return false;
            }
        } else if (!menuItem.isSeparator) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Non-separator item without action"));
            return false;
        }

        extension.items.append(menuItem);
        return true;
    };

    // Support either a single menu item or an array of items
    if (items.isArray()) {
        const quint32 length = items.property(QStringLiteral("length")).toUInt();
        for (quint32 i = 0; i < length; ++i)
            if (!addItem(items.property(i)))
                return;
    } else if (!addItem(items)) {
        return;
    }

    ActionManager::registerMenuExtension(menuId, extension);
}

QByteArray ScriptModule::compress(const QByteArray &data, CompressionMethod method, int compressionLevel)
{
    return Tiled::compress(data, static_cast<Tiled::CompressionMethod>(method), compressionLevel);
}

QByteArray ScriptModule::decompress(const QByteArray &data, CompressionMethod method)
{
    return Tiled::decompress(data, data.size(), static_cast<Tiled::CompressionMethod>(method));
}


void ScriptModule::trigger(const QByteArray &actionName) const
{
    if (QAction *action = ActionManager::findAction(actionName))
        action->trigger();
    else
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Unknown action"));
}

void ScriptModule::executeCommand(const QString &name, bool inTerminal) const
{
    const auto commands = CommandManager::instance()->allCommands();

    for (const Command &command : commands) {
        if (command.name == name) {
            command.execute(inTerminal);
            return;
        }
    }

    ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Unknown command"));
}

void ScriptModule::alert(const QString &text, const QString &title) const
{
    ScriptManager::ResetBlocker blocker;
    QMessageBox msgBox(QMessageBox::Warning, title, text, QMessageBox::Ok,
                       MainWindow::maybeInstance());

    // On macOS, QMessageBox hides the title by default
#ifdef Q_OS_MAC
    msgBox.QDialog::setWindowTitle(title);
#endif

    msgBox.exec();
}

bool ScriptModule::confirm(const QString &text, const QString &title) const
{
    ScriptManager::ResetBlocker blocker;
    QMessageBox msgBox(QMessageBox::Question, title, text,
                       QMessageBox::Yes | QMessageBox::No,
                       MainWindow::maybeInstance());

    // On macOS, QMessageBox hides the title by default
#ifdef Q_OS_MAC
    msgBox.QDialog::setWindowTitle(title);
#endif

    return msgBox.exec() == QMessageBox::Yes;
}

QString ScriptModule::prompt(const QString &label, const QString &text, const QString &title) const
{
    ScriptManager::ResetBlocker blocker;
    return QInputDialog::getText(MainWindow::maybeInstance(), title, label, QLineEdit::Normal, text);
}

QString ScriptModule::promptDirectory(const QString &defaultDir, const QString &title) const
{
    ScriptManager::ResetBlocker blocker;
    return QFileDialog::getExistingDirectory(MainWindow::maybeInstance(),
                                             title.isEmpty() ? tr("Open Directory") : title,
                                             defaultDir,
                                             QFileDialog::ShowDirsOnly);
}

QStringList ScriptModule::promptOpenFiles(const QString &defaultDir, const QString &filters, const QString &title) const
{
    ScriptManager::ResetBlocker blocker;
    return QFileDialog::getOpenFileNames(MainWindow::maybeInstance(),
                                         title.isEmpty() ? tr("Open Files") : title,
                                         defaultDir, filters);
}

QString ScriptModule::promptOpenFile(const QString &defaultDir, const QString &filters, const QString &title) const
{
    ScriptManager::ResetBlocker blocker;
    return QFileDialog::getOpenFileName(MainWindow::maybeInstance(),
                                        title.isEmpty() ? tr("Open File") : title,
                                        defaultDir, filters);
}

QString ScriptModule::promptSaveFile(const QString &defaultDir, const QString &filters, const QString &title) const
{
    ScriptManager::ResetBlocker blocker;
    return QFileDialog::getSaveFileName(MainWindow::maybeInstance(),
                                        title.isEmpty() ? tr("Save File") : title,
                                        defaultDir, filters);
}

void ScriptModule::log(const QString &text) const
{
    Tiled::INFO(text);
}

void ScriptModule::warn(const QString &text, QJSValue activated)
{
    Issue issue { Issue::Warning, text };
    setCallback(issue, activated);
    LoggingInterface::instance().report(issue);
}

void ScriptModule::error(const QString &text, QJSValue activated)
{
    Issue issue { Issue::Error, text };
    setCallback(issue, activated);
    LoggingInterface::instance().report(issue);
}

void ScriptModule::setCallback(Issue &issue, QJSValue activated)
{
    if (activated.isCallable()) {
        issue.setCallback([activated] () mutable {   // 'mutable' needed because of non-const QJSValue::call
            QJSValue result = activated.call();
            ScriptManager::instance().checkError(result);
        });
        issue.setContext(this);
    }
}

void ScriptModule::documentCreated(Document *document)
{
    emit assetCreated(document->editable());
}

void ScriptModule::documentOpened(Document *document)
{
    emit assetOpened(document->editable());
}

void ScriptModule::documentAboutToBeSaved(Document *document)
{
    emit assetAboutToBeSaved(document->editable());
}

void ScriptModule::documentSaved(Document *document)
{
    emit assetSaved(document->editable());
}

void ScriptModule::documentAboutToClose(Document *document)
{
    emit assetAboutToBeClosed(document->editable());
}

void ScriptModule::currentDocumentChanged(Document *document)
{
    emit activeAssetChanged(document ? document->editable() : nullptr);
}

QList<QObject *> ScriptModule::worlds() const
{
    QList<QObject*> worlds;

    auto documentManager = DocumentManager::maybeInstance();
    if (!documentManager)
        return worlds;

    for (const World *world : WorldManager::instance().worlds()) {
        WorldDocument *worldDocument = documentManager->ensureWorldDocument(world->fileName);
        worlds.append(worldDocument->editable());
    }

    return worlds;
}

void ScriptModule::loadWorld(const QString &fileName) const
{
    WorldManager::instance().loadWorld(fileName);
}

void ScriptModule::unloadWorld(const QString &fileName) const
{
    WorldManager::instance().unloadWorld(fileName);
}

void ScriptModule::unloadAllWorlds() const
{
    WorldManager::instance().unloadAllWorlds();
}

} // namespace Tiled

#include "moc_scriptmodule.cpp"
