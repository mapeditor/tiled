/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
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

#include "offsetmapdialog.h"

#include "mapdocument.h"
#include "tileselectionmodel.h"
#include "ui_offsetmapdialog.h"

using namespace Tiled::Internal;

OffsetMapDialog::OffsetMapDialog(QWidget *parent, MapDocument *mapDocument)
    : QDialog(parent)
{
    mUi = new Ui::OffsetMapDialog;
    mUi->setupUi(this);

    if (!mapDocument
        || !mapDocument->selectionModel()
        || mapDocument->selectionModel()->selection().isEmpty())
        disableBoundsSelectionCurrentArea();
}

OffsetMapDialog::~OffsetMapDialog()
{
    delete mUi;
}

OffsetMapDialog::LayerSelection OffsetMapDialog::layerSelection() const
{
    if (mUi->layerSelection->currentIndex() == 0)return ALL_VISIBLE_LAYERS;
    if (mUi->layerSelection->currentIndex() == 1)return ALL_LAYERS;
    return SELECTED_LAYERS;
}

OffsetMapDialog::BoundsSelection OffsetMapDialog::boundsSelection() const
{
    if (mUi->boundsSelection->currentIndex() == 0)return WHOLE_MAP;
    return CURRENT_SELECTION_AREA;
}

QPoint OffsetMapDialog::offset() const
{
    return QPoint(mUi->xOffset->value(), mUi->yOffset->value());
}

bool OffsetMapDialog::wrapX() const
{
    return mUi->wrapX->checkState();
}

bool OffsetMapDialog::wrapY() const
{
    return mUi->wrapY->checkState();
}

void OffsetMapDialog::disableBoundsSelectionCurrentArea()
{
    mUi->boundsSelection->setEnabled(false);
    mUi->boundsSelection->setCurrentIndex(0);
}
