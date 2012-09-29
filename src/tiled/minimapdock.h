/*
 * minimapdock.h
 * Copyright 2012, Christoph Schnackenberg <bluechs@gmx.de>
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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MINIMAPDOCK_H
#define MINIMAPDOCK_H

#include <QDockWidget>

namespace Tiled {
namespace Internal {

class MiniMap;
class MapDocument;
class MapView;

/**
 * Shows a mini-map.
 */
class MiniMapDock : public QDockWidget
{
    Q_OBJECT

public:
    MiniMapDock(QWidget *parent = 0);

    void setMapDocument(MapDocument *);

protected:
    void changeEvent(QEvent *e);

private:
    void retranslateUi();    

    MiniMap *mMiniMap;
};

} // namespace Internal
} // namespace Tiled

#endif // MINIMAPDOCK_H
