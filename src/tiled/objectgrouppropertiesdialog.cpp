/*
 * objectgrouppropertiesdialog.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Michael Woerister <michaelwoerister@gmail.com>
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

#include "objectgrouppropertiesdialog.h"

#include "mapdocument.h"
#include "objectgroup.h"
#include "colorbutton.h"
#include "changeobjectgroupproperties.h"

#include <QLabel>
#include <QLineEdit>
#include <QUndoStack>
#include <QGridLayout>
#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

ObjectGroupPropertiesDialog::ObjectGroupPropertiesDialog(
    MapDocument *mapDocument,
    ObjectGroup *objectGroup,
    QWidget *parent)

    : PropertiesDialog(tr("Object Layer"),
                       objectGroup,
                       mapDocument,
                       parent)
    , mObjectGroup(objectGroup)
    , mColorButton(new ColorButton)
{
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(new QLabel(tr("Color:")), 0, 0);
    grid->addWidget(mColorButton, 0, 1);

    mColorButton->setColor(mObjectGroup->color().isValid()
        ? mObjectGroup->color()
        : Qt::gray);

    qobject_cast<QBoxLayout*>(layout())->insertLayout(0, grid);

    connect(mColorButton, SIGNAL(colorChanged(QColor)),
            SLOT(colorChanged(QColor)));

    connect(mapDocument, SIGNAL(objectGroupChanged(ObjectGroup*)),
            SLOT(objectGroupChanged(ObjectGroup*)));
}

void ObjectGroupPropertiesDialog::colorChanged(const QColor &color)
{
    const QColor newColor = color != Qt::gray ? color : QColor();
    if (mObjectGroup->color() == newColor)
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->push(new ChangeObjectGroupProperties(mapDocument(),
                                                    mObjectGroup,
                                                    color));
}

void ObjectGroupPropertiesDialog::objectGroupChanged(ObjectGroup *objectGroup)
{
    if (mObjectGroup != objectGroup)
        return;

    mColorButton->setColor(objectGroup->color().isValid()
                           ? objectGroup->color()
                           : Qt::gray);
}
