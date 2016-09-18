/*
 * rtbinserttool.h
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#ifndef RTBINSERTTOOL_H
#define RTBINSERTTOOL_H

#include "stampbrush.h"

namespace Tiled {


namespace Internal {


class RTBInsertTool : public StampBrush
{
    Q_OBJECT

public:
    RTBInsertTool(QObject *parent = 0);
    ~RTBInsertTool();

    void languageChanged();

    void activate(MapScene *scene);

    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseMoved(const QPointF &pos, Qt::KeyboardModifiers modifiers);

signals:
    void cancelInsert();

protected:
    void tilePositionChanged(const QPoint &tilePos);
    void updateEnabledState();

private:

};

} // namespace Internal
} // namespace Tiled

#endif // RTBINSERTTOOL_H
