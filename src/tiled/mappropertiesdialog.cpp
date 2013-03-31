/*
 * mappropertiesdialog.cpp
 * Copyright 2012, Emmanuel Barroga emmanuelbarroga@gmail.com
 * Copyright 2012, Ben Longbons <b.r.longbons@gmail.com>
 * Copyright 2012-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "mappropertiesdialog.h"

#include "changemapproperties.h"
#include "colorbutton.h"
#include "map.h"
#include "mapdocument.h"

#include <QLabel>
#include <QLineEdit>
#include <QUndoStack>
#include <QGridLayout>
#include <QCoreApplication>
#include <QComboBox>

using namespace Tiled;
using namespace Tiled::Internal;

MapPropertiesDialog::MapPropertiesDialog(MapDocument *mapDocument,
                                         QWidget *parent)
    : PropertiesDialog(tr("Map"),
                       mapDocument->map(),
                       mapDocument,
                       parent)
    , mMapDocument(mapDocument)
    , mColorButton(new ColorButton)
    , mLayerDataCombo(new QComboBox)
{
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(new QLabel(tr("Background color:")), 0, 0);
    grid->addWidget(mColorButton, 0, 1);

    grid->addWidget(new QLabel(tr("Layer format:")), 1, 0);
    // fixme: is it possible to reuse the one from the main preferences?
    mLayerDataCombo->addItem(QLatin1String("Default"));
    mLayerDataCombo->addItem(QLatin1String("XML"));
    mLayerDataCombo->addItem(QLatin1String("Base64 (uncompressed)"));
    mLayerDataCombo->addItem(QLatin1String("Base64 (gzip compressed)"));
    mLayerDataCombo->addItem(QLatin1String("Base64 (zlib compressed)"));
    mLayerDataCombo->addItem(QLatin1String("CSV"));
    mLayerDataCombo->setCurrentIndex(mapDocument->map()->layerDataFormat() + 1);
    grid->addWidget(mLayerDataCombo);

    QColor bgColor = mapDocument->map()->backgroundColor();
    mColorButton->setColor(bgColor.isValid() ? bgColor : Qt::darkGray);

    qobject_cast<QBoxLayout*>(layout())->insertLayout(0, grid);

    connect(mLayerDataCombo, SIGNAL(currentIndexChanged(int)),
            SLOT(layerDataIndexChanged(int)));
    connect(mColorButton, SIGNAL(colorChanged(QColor)),
            SLOT(backgroundColorChanged(QColor)));

    connect(mapDocument, SIGNAL(mapChanged()),
            SLOT(mapChanged()));
}

void MapPropertiesDialog::layerDataIndexChanged(int index)
{
    if (index == -1) {
        // this shouldn't happen!
        index = 0;
    }

    const Map *map = mMapDocument->map();
    const Map::LayerDataFormat format = static_cast<Map::LayerDataFormat>(index - 1);
    if (map->layerDataFormat() == format)
        return;

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->push(new ChangeMapProperties(mMapDocument,
                                            map->backgroundColor(),
                                            format));
}

void MapPropertiesDialog::backgroundColorChanged(const QColor &color)
{
    QColor newColor = color;
    if (newColor == Qt::darkGray)
        newColor = QColor();

    const Map *map = mMapDocument->map();
    if (map->backgroundColor() == newColor)
        return;

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->push(new ChangeMapProperties(mMapDocument,
                                            newColor,
                                            map->layerDataFormat()));
}

void MapPropertiesDialog::mapChanged()
{
    const Map *map = mMapDocument->map();
    const QColor &bgColor = map->backgroundColor();

    mColorButton->setColor(bgColor.isValid() ? bgColor : Qt::darkGray);
    mLayerDataCombo->setCurrentIndex(map->layerDataFormat() + 1);
}
