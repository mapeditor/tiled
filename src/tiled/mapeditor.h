/*
 * mapeditor.h
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindijer.nl>
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

#ifndef TILED_INTERNAL_MAPEDITOR_H
#define TILED_INTERNAL_MAPEDITOR_H

#include <QMainWindow>
#include <QHash>

class QStackedWidget;

namespace Tiled {
namespace Internal {

class AbstractTool;
class MapDocument;
class MapView;
class MapViewContainer;

class LayerDock;

class MapEditor : public QMainWindow
{
    Q_OBJECT

public:
    explicit MapEditor(QWidget *parent = nullptr);

    void addMapDocument(MapDocument *mapDocument);
    void removeMapDocument(MapDocument *mapDocument);
    void setCurrentMapDocument(MapDocument *mapDocument);

signals:

public slots:

private:
    LayerDock *mLayerDock;
    QStackedWidget *mWidgetStack;
    QHash<MapDocument*, MapViewContainer*> mWidgetForMap;

    AbstractTool *mSelectedTool;
    MapView *mViewWithTool;
};

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_MAPEDITOR_H
