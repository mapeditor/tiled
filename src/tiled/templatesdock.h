/*
 * templatesdock.h
 * Copyright 2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Mohamed Thabet <thabetx@gmail.com>
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

#include <QDockWidget>
#include <QTreeView>
#include <QAction>

namespace Tiled {

class ObjectTemplate;
class MapObject;
class Tile;

namespace Internal {

class AbstractTool;
class MapDocument;
class MapScene;
class MapView;
class ObjectTemplateModel;
class PropertiesDock;
class TemplatesView;
class ToolManager;

class TemplatesDock : public QDockWidget
{
    Q_OBJECT

public:
    TemplatesDock(QWidget *parent = nullptr);
    ~TemplatesDock();

    void setPropertiesDock(PropertiesDock *propertiesDock);

signals:
    void currentTemplateChanged(ObjectTemplate *objectTemplate);
    void templateEdited(const ObjectTemplate *objectTemplate);
    void setTile(Tile *tile);

private slots:
    void setSelectedTool(AbstractTool *tool);
    void setTemplate(ObjectTemplate *objectTemplate);

    void undo();
    void redo();
    void applyChanges();

    void chooseDirectory();

protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void retranslateUi();

    TemplatesView *mTemplatesView;

    QAction *mChooseDirectory;
    QAction *mUndoAction;
    QAction *mRedoAction;

    MapDocument *mDummyMapDocument;
    MapScene *mMapScene;
    MapView *mMapView;
    ObjectTemplate *mObjectTemplate;
    MapObject *mObject;
    PropertiesDock *mPropertiesDock;
    ToolManager *mToolManager;
};

class TemplatesView : public QTreeView
{
    Q_OBJECT

public:
    QSize sizeHint() const override;
    TemplatesView(QWidget *parent = nullptr);

signals:
    void currentTemplateChanged(ObjectTemplate *objectTemplate);
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

public slots:
    void onCurrentChanged(const QModelIndex &index);

private:
    void onTemplatesDirectoryChanged(const QString &templatesDirectory);

    ObjectTemplateModel *mModel;
};

inline void TemplatesDock::setPropertiesDock(PropertiesDock *propertiesDock)
{ mPropertiesDock = propertiesDock; }

} // namespace Internal
} // namespace Tiled
