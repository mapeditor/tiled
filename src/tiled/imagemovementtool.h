/*
* imagemovementtool.h
* Copyright 2014, Mattia Basaglia
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

#ifndef IMAGEMOVEMENTTOOL_H
#define IMAGEMOVEMENTTOOL_H

#include "abstractimagetool.h"

namespace Tiled {
namespace Internal {

class ImageMovementTool : public AbstractImageTool
{
    Q_OBJECT

public:
    explicit ImageMovementTool(QObject *parent = nullptr);

    void activate(MapScene *scene);
    void deactivate(MapScene *scene);
    void mouseEntered();
    void mouseMoved(const QPointF &pos,
                    Qt::KeyboardModifiers modifiers);
    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);

    void languageChanged();

private:
    bool mMousePressed;
    QPointF mMouseStart;
    QPoint mLayerStart;
};

} // namespace Internal
} // namespace Tiled

#endif // IMAGEMOVEMENTTOOL_HPP
