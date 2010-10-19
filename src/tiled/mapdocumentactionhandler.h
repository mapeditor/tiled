/*
 * mapdocumentactionhandler.h
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef MAPDOCUMENTACTIONHANDLER_H
#define MAPDOCUMENTACTIONHANDLER_H

#include <QObject>

class QAction;

namespace Tiled {
namespace Internal {

class MapDocument;

/**
 * The map document action handler deals with most basic actions that can be
 * performed on a MapDocument.
 */
class MapDocumentActionHandler : public QObject
{
    Q_OBJECT

public:
    explicit MapDocumentActionHandler(QObject *parent = 0);
    ~MapDocumentActionHandler();

    static MapDocumentActionHandler *instance() { return mInstance; }

    void retranslateUi();

    void setMapDocument(MapDocument *mapDocument);
    MapDocument *mapDocument() const { return mMapDocument; }

    QAction *actionSelectAll() const { return mActionSelectAll; }
    QAction *actionSelectNone() const { return mActionSelectNone; }
    QAction *actionAddTileLayer() const { return mActionAddTileLayer; }
    QAction *actionAddObjectGroup() const { return mActionAddObjectGroup; }
    QAction *actionDuplicateLayer() const { return mActionDuplicateLayer; }
    QAction *actionRemoveLayer() const { return mActionRemoveLayer; }
    QAction *actionMoveLayerUp() const { return mActionMoveLayerUp; }
    QAction *actionMoveLayerDown() const { return mActionMoveLayerDown; }
    QAction *actionLayerProperties() const { return mActionLayerProperties; }

signals:
    void mapDocumentChanged(MapDocument *mapDocument);

public slots:
    void selectAll();
    void selectNone();

    void addTileLayer();
    void addObjectGroup();
    void duplicateLayer();
    void moveLayerUp();
    void moveLayerDown();
    void removeLayer();

private slots:
    void updateActions();

private:
    MapDocument *mMapDocument;

    QAction *mActionSelectAll;
    QAction *mActionSelectNone;
    QAction *mActionAddTileLayer;
    QAction *mActionAddObjectGroup;
    QAction *mActionDuplicateLayer;
    QAction *mActionRemoveLayer;
    QAction *mActionMoveLayerUp;
    QAction *mActionMoveLayerDown;
    QAction *mActionLayerProperties;

    static MapDocumentActionHandler *mInstance;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPDOCUMENTACTIONHANDLER_H
