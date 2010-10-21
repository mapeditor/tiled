/*
 * tmxviewer.h
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

#ifndef TMXVIEWER_H
#define TMXVIEWER_H

#include <QGraphicsView>

namespace Tiled {
class Map;
class MapRenderer;
}

class TmxViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit TmxViewer(QWidget *parent = 0);
    ~TmxViewer();

    void viewMap(const QString &fileName);

private:
    QGraphicsScene *mScene;
    Tiled::Map *mMap;
    Tiled::MapRenderer *mRenderer;
};

#endif // TMXVIEWER_H
