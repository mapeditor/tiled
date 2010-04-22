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
#include "movemapobject.h"
#include "resizemapobject.h"

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
    , mObjectPropertiesDialog(new Ui::ObjectPropertiesDialog)
{
    QWidget *widget = new QWidget;
    mObjectPropertiesDialog->setupUi(widget);

    // Initialize UI with values from the map-object
    mObjectPropertiesDialog->name->setText(mMapObject->name());
    mObjectPropertiesDialog->type->setText(mMapObject->type());
    mObjectPropertiesDialog->x->setValue(mMapObject->x());
    mObjectPropertiesDialog->y->setValue(mMapObject->y());
    mObjectPropertiesDialog->width->setValue(mMapObject->width());
    mObjectPropertiesDialog->height->setValue(mMapObject->height());

    qobject_cast<QBoxLayout*>(this->layout())->insertWidget(0, widget);

    mObjectPropertiesDialog->name->setFocus();
}

void ObjectPropertiesDialog::accept()
{
    const QString newName = mObjectPropertiesDialog->name->text();
    const QString newType = mObjectPropertiesDialog->type->text();

    const qreal newPosX = mObjectPropertiesDialog->x->value();
    const qreal newPosY = mObjectPropertiesDialog->y->value();
    const qreal newWidth = mObjectPropertiesDialog->width->value();
    const qreal newHeight = mObjectPropertiesDialog->height->value();

    bool changed = false;
    changed |= mMapObject->name() != newName;
    changed |= mMapObject->type() != newType;
    changed |= mMapObject->x() != newPosX;
    changed |= mMapObject->y() != newPosY;
    changed |= mMapObject->width() != newWidth;
    changed |= mMapObject->height() != newHeight;

    if (changed) {
        QUndoStack *undo = mMapDocument->undoStack();
        undo->beginMacro(tr("Change Object"));
        undo->push(new ChangeMapObject(mMapDocument, mMapObject,
                                       newName, newType));

        const QPointF oldPos = mMapObject->position();
        mMapObject->setX(newPosX);
        mMapObject->setY(newPosY);
        undo->push(new MoveMapObject(mMapDocument, mMapObject, oldPos));

        const QSizeF oldSize = mMapObject->size();
        mMapObject->setWidth(newWidth);
        mMapObject->setHeight(newHeight);
        undo->push(new ResizeMapObject(mMapDocument, mMapObject, oldSize));

        PropertiesDialog::accept(); // Let PropertiesDialog add its command
        undo->endMacro();

        mMapDocument->emitObjectChanged(mMapObject);
    } else {
        PropertiesDialog::accept();
    }
}
