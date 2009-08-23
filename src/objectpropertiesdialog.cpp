/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
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

#include "objectpropertiesdialog.h"

#include "changemapobject.h"
#include "mapdocument.h"
#include "mapobject.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

ObjectPropertiesDialog::ObjectPropertiesDialog(MapDocument *mapDocument,
                                               MapObject *mapObject,
                                               QWidget *parent)
    : PropertiesDialog(tr("Object"),
                       mapObject->properties(),
                       mapDocument->undoStack(),
                       parent)
    , mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mNameEdit(new QLineEdit(mMapObject->name()))
    , mTypeEdit(new QLineEdit(mMapObject->type()))
{
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(new QLabel(tr("Name:")), 0, 0);
    grid->addWidget(new QLabel(tr("Type:")), 1, 0);
    grid->addWidget(mNameEdit, 0, 1);
    grid->addWidget(mTypeEdit, 1, 1);
    qobject_cast<QBoxLayout*>(layout())->insertLayout(0, grid);

    mNameEdit->setFocus();
}

void ObjectPropertiesDialog::accept()
{
    const QString newName = mNameEdit->text();
    const QString newType = mTypeEdit->text();

    if (mMapObject->name() != newName || mMapObject->type() != newType) {
        QUndoStack *undo = mMapDocument->undoStack();
        undo->beginMacro(tr("Change Object"));
        undo->push(new ChangeMapObject(mMapDocument, mMapObject,
                                       newName, newType));
        PropertiesDialog::accept(); // Let PropertiesDialog add its command
        undo->endMacro();
    } else {
        PropertiesDialog::accept();
    }
}
