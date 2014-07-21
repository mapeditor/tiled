/*
 * abstractimagetool.h
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

#ifndef ABSTRACTIMAGETOOL_H
#define ABSTRACTIMAGETOOL_H

#include "abstracttool.h"

namespace Tiled {

class ImageLayer;

namespace Internal {


/**
 * A convenient base class for tools that work on image layers.
 */
class AbstractImageTool : public AbstractTool
{
    Q_OBJECT

public:
    /**
     * Constructs an abstract image tool with the given \a name and \a icon.
     */
    AbstractImageTool(const QString &name,
                       const QIcon &icon,
                       const QKeySequence &shortcut,
                       QObject *parent = 0);

    void activate(MapScene *scene);
    void deactivate(MapScene *scene);

    void keyPressed(QKeyEvent *event);
    void mouseLeft();
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers);

protected:
    void updateEnabledState();

    MapScene *mapScene() const { return mMapScene; }
    ImageLayer *currentImageLayer() const;

private:
    MapScene *mMapScene;
};

} // namespace Internal
} // namespace Tiled

#endif // ABSTRACTIMAGETOOL_H
