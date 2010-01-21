/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef ERASER_H
#define ERASER_H

#include "abstracttiletool.h"

namespace Tiled {
namespace Internal {

/**
 * Implements a simple eraser tool.
 */
class Eraser : public AbstractTileTool
{
    Q_OBJECT

public:
    Eraser(QObject *parent = 0);

    void tilePositionChanged(const QPoint &tilePos);

    void mousePressed(const QPointF &pos, Qt::MouseButton button,
                      Qt::KeyboardModifiers modifiers);
    void mouseReleased(const QPointF &pos, Qt::MouseButton button);

    void languageChanged();

private:
    void doErase(bool mergeable);

    bool mErasing;
};

} // namespace Internal
} // namespace Tiled

#endif // ERASER_H
