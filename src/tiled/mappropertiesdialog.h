/*
 * mappropertiesdialog.h
 * Copyright 2012, Emmanuel Barroga emmanuelbarroga@gmail.com
 * Copyright 2012, Ben Longbons <b.r.longbons@gmail.com>
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef MAPPROPERTIESDIALOG_H
#define MAPPROPERTIESDIALOG_H

#include "propertiesdialog.h"

class QComboBox;

namespace Tiled {

namespace Internal {

class ColorButton;

class MapPropertiesDialog : public PropertiesDialog
{
    Q_OBJECT

public:
    MapPropertiesDialog(MapDocument *mapDocument, QWidget *parent = 0);

private slots:
    void layerDataIndexChanged(int);
    void backgroundColorChanged(const QColor &);
    void mapChanged();

private:
    MapDocument *mMapDocument;
    ColorButton *mColorButton;
    QComboBox *mLayerDataCombo;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPPROPERTIESDIALOG_H
