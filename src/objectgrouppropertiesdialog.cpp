/*
 * Tiled Map Editor (Qt)
 * Copyright 2010 Tiled (Qt) developers (see AUTHORS file)
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
                       objectGroup->properties(),
                       mapDocument->undoStack(),
                       parent)
    , mMapDocument(mapDocument)
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
}

void ObjectGroupPropertiesDialog::accept()
{
    QUndoStack *undoStack = mMapDocument->undoStack();

    const QColor newColor = mColorButton->color() != Qt::gray
        ? mColorButton->color()
        : QColor();

    const bool localChanges = newColor != mObjectGroup->color();

    if (localChanges) {
        undoStack->beginMacro(QCoreApplication::translate(
            "Undo Commands",
            "Change Object Layer Properties"));

        undoStack->push(new ChangeObjectGroupProperties(
            mMapDocument,
            mObjectGroup,
            mColorButton->color()));
    }

    PropertiesDialog::accept();

    if (localChanges)
        undoStack->endMacro();
}
