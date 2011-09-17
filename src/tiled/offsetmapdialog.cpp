/*
 * offsetmapdialog.cpp
 * Copyright 2009, Jeff Bland <jeff@teamphobic.com>
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "offsetmapdialog.h"

#include "map.h"
#include "mapdocument.h"
#include "tilelayer.h"
#include "ui_offsetmapdialog.h"

using namespace Tiled::Internal;

OffsetMapDialog::OffsetMapDialog(MapDocument *mapDocument, QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::OffsetMapDialog)
    , mMapDocument(mapDocument)
{
    mUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    if (mMapDocument->tileSelection().isEmpty())
        disableBoundsSelectionCurrentArea();
    else
        mUi->boundsSelection->setCurrentIndex(1);
}

OffsetMapDialog::~OffsetMapDialog()
{
    delete mUi;
}

QList<int> OffsetMapDialog::affectedLayerIndexes() const
{
    QList<int> layerIndexes;
    const Map *map = mMapDocument->map();

    switch (layerSelection()) {
    case AllVisibleLayers:
        for (int i = 0; i < map->layerCount(); i++)
            if (map->layerAt(i)->isVisible())
                layerIndexes.append(i);
        break;
    case AllLayers:
        for (int i = 0; i < map->layerCount(); i++)
            layerIndexes.append(i);
        break;
    case SelectedLayer:
        layerIndexes.append(mMapDocument->currentLayerIndex());
        break;
    }

    return layerIndexes;
}

QRect OffsetMapDialog::affectedBoundingRect() const
{
    QRect boundingRect;

    switch (boundsSelection()) {
    case WholeMap:
        boundingRect = QRect(QPoint(0, 0), mMapDocument->map()->size());
        break;
    case CurrentSelectionArea: {
        const QRegion &selection = mMapDocument->tileSelection();

        Q_ASSERT_X(!selection.isEmpty(),
                   "OffsetMapDialog::affectedBoundingRect()",
                   "selection is empty");

        boundingRect = selection.boundingRect();
        break;
    }
    }

    return boundingRect;
}

OffsetMapDialog::LayerSelection OffsetMapDialog::layerSelection() const
{
    switch (mUi->layerSelection->currentIndex()) {
    case 0:
        return AllVisibleLayers;
    case 1:
        return AllLayers;
    default:
        return SelectedLayer;
    }
}

OffsetMapDialog::BoundsSelection OffsetMapDialog::boundsSelection() const
{
    if (mUi->boundsSelection->currentIndex() == 0)
        return WholeMap;
    return CurrentSelectionArea;
}

QPoint OffsetMapDialog::offset() const
{
    return QPoint(mUi->xOffset->value(), mUi->yOffset->value());
}

bool OffsetMapDialog::wrapX() const
{
    return mUi->wrapX->isChecked();
}

bool OffsetMapDialog::wrapY() const
{
    return mUi->wrapY->isChecked();
}

void OffsetMapDialog::disableBoundsSelectionCurrentArea()
{
    mUi->boundsSelection->setEnabled(false);
    mUi->boundsSelection->setCurrentIndex(0);
}
