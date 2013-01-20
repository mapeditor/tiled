/*
 * mappropertiesdialog.cpp
 * Copyright 2012, Emmanuel Barroga emmanuelbarroga@gmail.com
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
                       mapDocument->undoStack(),
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
    mLayerDataCombo->setCurrentIndex(mMapDocument->map()->layerDataFormat() + 1);
    grid->addWidget(mLayerDataCombo);

    QColor bgColor = mapDocument->map()->backgroundColor();
    mColorButton->setColor(bgColor.isValid() ? bgColor : Qt::darkGray);

    qobject_cast<QBoxLayout*>(layout())->insertLayout(0, grid);
}

void MapPropertiesDialog::accept()
{
    int format = mLayerDataCombo->currentIndex();
    if (format == -1) {
        // this shouldn't happen!
        format = 0;
    }

    Map::LayerDataFormat newLayerDataFormat = static_cast<Map::LayerDataFormat>(format - 1);

    QUndoStack *undoStack = mMapDocument->undoStack();

    QColor newColor = mColorButton->color();
    if (newColor == Qt::darkGray)
        newColor = QColor();

    const Map *map = mMapDocument->map();
    bool localChanges = newColor != map->backgroundColor() ||
            newLayerDataFormat != map->layerDataFormat();

    if (localChanges) {
        undoStack->beginMacro(QCoreApplication::translate(
            "Undo Commands",
            "Change Map Properties"));

        undoStack->push(new ChangeMapProperties(mMapDocument,
                                                newColor,
                                                newLayerDataFormat));
    }

    PropertiesDialog::accept();

    if (localChanges)
        undoStack->endMacro();
}
