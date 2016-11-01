/*
 * rtbtilebutton.h
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

#ifndef RTBTOOLBUTTON_H
#define RTBTOOLBUTTON_H

#include "abstracttiletool.h"
#include <QObject>

namespace Tiled {

namespace Internal {


class RTBTileButton : public AbstractTileTool
{
    Q_OBJECT

public:
    RTBTileButton(QObject *parent, int type, int layerType);
    ~RTBTileButton();

    void languageChanged();

    void mousePressed(QGraphicsSceneMouseEvent *event);
    void mouseReleased(QGraphicsSceneMouseEvent *event);

    int type() const;

    void setVisible(bool visible);
    bool isVisible()  { return mVisible; }

public slots:
    void updateTooltip();

signals:
    void visibleChanged(bool visible);

protected:
    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument);
    void updateEnabledState();
    void updateVisibleState();

    void tilePositionChanged(const QPoint &tilePos);

private:
    int mType;
    int mLayerType;
    bool mVisible;

};

} // namespace Internal
} // namespace Tiled

#endif // RTBTOOLBUTTON_H
