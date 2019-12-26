/*
 * scriptmodule.h
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

#pragma once

#include "documentmanager.h"
#include "id.h"
#include "issuesdock.h"

#include <QJSValue>
#include <QObject>
#include <QVector>

#include <map>
#include <memory>

class QAction;

namespace Tiled {

class EditableAsset;
class MapEditor;
class ScriptedAction;
class ScriptedMapFormat;
class ScriptedTilesetFormat;
class ScriptedTool;
class TilesetEditor;

/**
 * Initial point of access to Tiled functionality from JavaScript.
 */
class ScriptModule : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString version READ version)
    Q_PROPERTY(QString platform READ platform)
    Q_PROPERTY(QString arch READ arch)

    Q_PROPERTY(QStringList actions READ actions)
    Q_PROPERTY(QStringList menus READ menus)

    Q_PROPERTY(Tiled::EditableAsset *activeAsset READ activeAsset WRITE setActiveAsset NOTIFY activeAssetChanged)
    Q_PROPERTY(QList<QObject*> openAssets READ openAssets)

    Q_PROPERTY(Tiled::MapEditor *mapEditor READ mapEditor)
    Q_PROPERTY(Tiled::TilesetEditor *tilesetEditor READ tilesetEditor)

public:
    struct MenuItem {
        Id action;
        Id beforeAction;
        bool isSeparator;
    };

    struct MenuExtension {
        Id menuId;
        QVector<MenuItem> items;
    };

    ScriptModule(QObject *parent = nullptr);
    ~ScriptModule() override;

    QString version() const;
    QString platform() const;
    QString arch() const;

    QStringList actions() const;
    QStringList menus() const;

    EditableAsset *activeAsset() const;
    bool setActiveAsset(EditableAsset *asset) const;

    QList<QObject*> openAssets() const;

    TilesetEditor *tilesetEditor() const;
    MapEditor *mapEditor() const;

    Q_INVOKABLE Tiled::EditableAsset *open(const QString &fileName) const;
    Q_INVOKABLE bool close(Tiled::EditableAsset *asset) const;
    Q_INVOKABLE Tiled::EditableAsset *reload(Tiled::EditableAsset *asset) const;

    Q_INVOKABLE Tiled::ScriptedAction *registerAction(const QByteArray &id, QJSValue callback);
    Q_INVOKABLE void registerMapFormat(const QString &shortName, QJSValue mapFormatObject);
    Q_INVOKABLE void registerTilesetFormat(const QString &shortName, QJSValue tilesetFormatObject);
    Q_INVOKABLE QJSValue registerTool(const QString &shortName, QJSValue toolObject);

    Q_INVOKABLE void extendMenu(const QByteArray &idName, QJSValue items);

signals:
    void assetCreated(Tiled::EditableAsset *asset);
    void assetOpened(Tiled::EditableAsset *asset);
    void assetAboutToBeSaved(Tiled::EditableAsset *asset);
    void assetSaved(Tiled::EditableAsset *asset);
    void assetAboutToBeClosed(Tiled::EditableAsset *asset);

    void activeAssetChanged(Tiled::EditableAsset *asset);

public slots:
    void trigger(const QByteArray &actionName) const;
    void executeCommand(const QString &name, bool inTerminal = false) const;

    void alert(const QString &text, const QString &title = QString()) const;
    bool confirm(const QString &text, const QString &title = QString()) const;
    QString prompt(const QString &label, const QString &text = QString(), const QString &title = QString()) const;

    void log(const QString &text) const;

    void warn(const QString &text, QJSValue activated = QJSValue());
    void error(const QString &text, QJSValue activated = QJSValue());

private:
    void documentCreated(Document *document);
    void documentOpened(Document *document);
    void documentAboutToBeSaved(Document *document);
    void documentSaved(Document *document);
    void documentAboutToClose(Document *document);
    void currentDocumentChanged(Document *document);

    void setCallback(Issue &issue, QJSValue activated);

    std::map<Id, std::unique_ptr<ScriptedAction>> mRegisteredActions;
    std::map<QString, std::unique_ptr<ScriptedMapFormat>> mRegisteredMapFormats;
    std::map<QString, std::unique_ptr<ScriptedTilesetFormat>> mRegisteredTilesetFormats;
    std::map<Id, std::unique_ptr<ScriptedTool>> mRegisteredTools;

    QVector<MenuExtension> mMenuExtensions;
};

} // namespace Tiled
