/*
 * bucketfilltool.h
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
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

#pragma once

#include "abstracttilefilltool.h"
#include "tilelayer.h"
#include "tilestamp.h"

namespace Tiled {

class WangSet;

namespace Internal {

class MapDocument;
class StampActions;
class WangFiller;

/**
 * Implements a tool that bucket fills (flood fills) a region with a repeatable
 * stamp.
 */
class BucketFillTool : public AbstractTileFillTool
{
    Q_OBJECT

public:
    BucketFillTool(QObject *parent = nullptr);
    ~BucketFillTool() override;

    void mousePressed(QGraphicsSceneMouseEvent *event) override;

    void modifiersChanged(Qt::KeyboardModifiers) override;

    void languageChanged() override;

protected:
    void tilePositionChanged(const QPoint &tilePos) override;
    void clearConnections(MapDocument *mapDocument) override;

private slots:
    void clearOverlay();

private:
    bool mLastShiftStatus;

    void makeConnections();
};

} // namespace Internal
} // namespace Tiled
