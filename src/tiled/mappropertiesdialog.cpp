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
{
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(new QLabel(tr("Background color:")), 0, 0);
    grid->addWidget(mColorButton, 0, 1);

    QColor bgColor = mapDocument->map()->backgroundColor();
    mColorButton->setColor(bgColor.isValid() ? bgColor : Qt::darkGray);

    qobject_cast<QBoxLayout*>(layout())->insertLayout(0, grid);
}

void MapPropertiesDialog::accept()
{
    QUndoStack *undoStack = mMapDocument->undoStack();

    QColor newColor = mColorButton->color();
    if (newColor == Qt::darkGray)
        newColor = QColor();

    const Map *map = mMapDocument->map();
    const bool localChanges = newColor != map->backgroundColor();

    if (localChanges) {
        undoStack->beginMacro(QCoreApplication::translate(
            "Undo Commands",
            "Change Map Properties"));

        undoStack->push(new ChangeMapProperties(mMapDocument, newColor));
    }

    PropertiesDialog::accept();

    if (localChanges)
        undoStack->endMacro();
}
