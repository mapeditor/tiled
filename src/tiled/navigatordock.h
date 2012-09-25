/*
 * navigatordock.h
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

#ifndef NAVIGATORDOCK_H
#define NAVIGATORDOCK_H

#include <QDockWidget>
#include <QTimer>

namespace Tiled {
namespace Internal {

class NavigatorFrame;
class MapDocument;
class MapView;

/**
 *
 */
class NavigatorDock: public QDockWidget
{
    Q_OBJECT

public:

    NavigatorDock(QWidget* parent=0);
    void setMapDocument(MapDocument*);
    /** should be notified whenever zoom/scrollpos has changed */
    void mapViewChanged();
    /** Should be notified whenever the content of the map has changed */
    void mapModelChanged(bool buffered);

protected:

    void changeEvent(QEvent *e);

private slots:

    void redrawTimeout();

private:

    void retranslateUi();    

    NavigatorFrame *mDrawFrame;
    MapDocument *mMapDocument;
    QTimer mUpdateSuspendTimer;

};


} // namespace Internal
} // namespace Tiled

#endif // NAVIGATORDOCK_H 
