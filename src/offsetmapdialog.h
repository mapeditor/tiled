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

#ifndef OFFSETMAPDIALOG_H
#define OFFSETMAPDIALOG_H

#include <QDialog>

namespace Ui {
class OffsetMapDialog;
}

namespace Tiled {
namespace Internal {

class MapDocument;

class OffsetMapDialog : public QDialog
{
    Q_OBJECT

public:
    OffsetMapDialog(MapDocument *mapDocument, QWidget *parent = 0);

    ~OffsetMapDialog();

    QList<int> affectedLayerIndexes() const;
    QRect affectedBoundingRect() const;

    QPoint offset() const;
    bool wrapX() const;
    bool wrapY() const;

private:
    enum LayerSelection {
        AllVisibleLayers,
        AllLayers,
        SelectedLayer
    };

    enum BoundsSelection {
        WholeMap,
        CurrentSelectionArea
    };

    LayerSelection layerSelection() const;
    BoundsSelection boundsSelection() const;

    void disableBoundsSelectionCurrentArea();

    Ui::OffsetMapDialog *mUi;
    MapDocument *mMapDocument;
};

} // namespace Internal
} // namespace Tiled

#endif // OFFSETMAPDIALOG_H
