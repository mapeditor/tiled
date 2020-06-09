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

#include "mapdocument.h"

#include <QDockWidget>
#include <QHash>

class QAction;
class QPushButton;
class QLabel;

namespace Tiled {

class ObjectTemplate;
class MapObject;
class Tile;

class AbstractTool;
class MapScene;
class MapView;
class PropertiesDock;
class TemplatesView;
class ToolManager;

class TemplatesDock : public QDockWidget
{
    Q_OBJECT

public:
    TemplatesDock(QWidget *parent = nullptr);
    ~TemplatesDock() override;

    void setPropertiesDock(PropertiesDock *propertiesDock);
    void setTile(Tile *tile);

signals:
    void currentTemplateChanged(ObjectTemplate *objectTemplate);
    void templateTilesetReplaced();

public slots:
    void openTemplate(const QString &path);
    void tryOpenTemplate(const QString &filePath);
    void bringToFront();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void setTemplate(ObjectTemplate *objectTemplate);
    void checkTileset();

    void undo();
    void redo();
    void applyChanges();

    void retranslateUi();
    void fixTileset();

    MapObject *dummyObject() const;

    QAction *mUndoAction;
    QAction *mRedoAction;
    QPushButton *mFixTilesetButton;
    QLabel *mDescriptionLabel;

    MapDocumentPtr mDummyMapDocument;
    MapScene *mMapScene;
    MapView *mMapView;
    ObjectTemplate *mObjectTemplate;
    PropertiesDock *mPropertiesDock;
    ToolManager *mToolManager;

    static QHash<ObjectTemplate*, QWeakPointer<MapDocument>> ourDummyDocuments;
};

inline void TemplatesDock::setPropertiesDock(PropertiesDock *propertiesDock)
{ mPropertiesDock = propertiesDock; }

} // namespace Tiled
