/*
 * bucketfilltool.h
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
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

#ifndef BUCKETFILLTOOL_H
#define BUCKETFILLTOOL_H

#include "abstracttiletool.h"

namespace Tiled {

class TileLayer;

namespace Internal {

class MapDocument;

/**
 * Implements a tool that bucket fills (flood fills) a region with a repeatable
 * stamp.
 */
class BucketFillTool : public AbstractTileTool
{
    Q_OBJECT

public:
    BucketFillTool(QObject *parent = 0);
    ~BucketFillTool();

    void activate(MapScene *scene);

    void mousePressed(const QPointF &pos, Qt::MouseButton button,
                      Qt::KeyboardModifiers modifiers);
    void mouseReleased(const QPointF &pos, Qt::MouseButton button);

    void modifiersChanged(Qt::KeyboardModifiers);

    void languageChanged();

    /**
     * Sets the stamp that is drawn when filling. The BucketFillTool takes
     * ownership over the stamp layer.
     */
    void setStamp(TileLayer *stamp);

protected:
    void tilePositionChanged(const QPoint &tilePos);

    void mapDocumentChanged(MapDocument *oldDocument,
                            MapDocument *newDocument);

private slots:
    void clearOverlay();

private:
    void makeConnections();
    void clearConnections(MapDocument *mapDocument);

    TileLayer *mStamp;
    TileLayer *mFillOverlay;
    QRegion mFillRegion;

    bool mLastShiftStatus;
};

} // namespace Internal
} // namespace Tiled

#endif // BUCKETFILLTOOL_H
