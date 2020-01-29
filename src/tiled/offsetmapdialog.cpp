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

namespace Tiled {

OffsetMapDialog::OffsetMapDialog(MapDocument *mapDocument, QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::OffsetMapDialog)
    , mMapDocument(mapDocument)
{
    mUi->setupUi(this);
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif

    if (mMapDocument->selectedArea().isEmpty()) {
        setBoundsSelection(WholeMap);
        mUi->boundsSelection->setEnabled(false);
    } else {
        setBoundsSelection(CurrentSelectionArea);
    }

    boundsSelectionChanged();   // updates wrap checkboxes

    connect(mUi->boundsSelection, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &OffsetMapDialog::boundsSelectionChanged);
}

OffsetMapDialog::~OffsetMapDialog()
{
    delete mUi;
}

QList<Layer *> OffsetMapDialog::affectedLayers() const
{
    QList<Layer *> layers;

    LayerIterator iterator(mMapDocument->map());

    switch (layerSelection()) {
    case AllVisibleLayers:
        while (Layer *layer = iterator.next())
            if (!layer->isGroupLayer() && layer->isVisible())
                layers.append(layer);
        break;
    case AllLayers:
        while (Layer *layer = iterator.next())
            if (!layer->isGroupLayer())
                layers.append(layer);
        break;
    case SelectedLayers:
        for (Layer *layer : mMapDocument->selectedLayers())
            if (!layer->isGroupLayer())
                layers.append(layer);
        break;
    }

    return layers;
}

/**
 * Returns the bounding rect that is to be affected by the offset operation.
 *
 * For infinite maps, when not using the currently selected area, the returned
 * rect is empty.
 */
QRect OffsetMapDialog::affectedBoundingRect() const
{
    QRect boundingRect;

    switch (boundsSelection()) {
    case WholeMap:
        if (!mMapDocument->map()->infinite())
            boundingRect = QRect(QPoint(0, 0), mMapDocument->map()->size());
        break;
    case CurrentSelectionArea: {
        const QRegion &selection = mMapDocument->selectedArea();

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
        return SelectedLayers;
    }
}

OffsetMapDialog::BoundsSelection OffsetMapDialog::boundsSelection() const
{
    if (mUi->boundsSelection->currentIndex() == 0)
        return WholeMap;
    return CurrentSelectionArea;
}

void OffsetMapDialog::setBoundsSelection(BoundsSelection boundsSelection)
{
    switch (boundsSelection) {
    case WholeMap:
        mUi->boundsSelection->setCurrentIndex(0);
        break;
    case CurrentSelectionArea:
        mUi->boundsSelection->setCurrentIndex(1);
        break;
    }
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

void OffsetMapDialog::boundsSelectionChanged()
{
    bool wrapEnabled = true;

    if (boundsSelection() == WholeMap && mMapDocument->map()->infinite())
        wrapEnabled = false;

    mUi->wrapX->setEnabled(wrapEnabled);
    mUi->wrapY->setEnabled(wrapEnabled);

    if (!wrapEnabled) {
        mUi->wrapX->setChecked(false);
        mUi->wrapY->setChecked(false);
    }
}

} // namespace Tiled
