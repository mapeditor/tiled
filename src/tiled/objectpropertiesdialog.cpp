/*
 * objectpropertiesdialog.cpp
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

#include "objectpropertiesdialog.h"
#include "ui_objectpropertiesdialog.h"

#include "changemapobject.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "movemapobject.h"
#include "objecttypesmodel.h"
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
                       mapObject,
                       mapDocument->undoStack(),
                       parent)
    , mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mUi(new Ui::ObjectPropertiesDialog)
{
    QWidget *widget = new QWidget;
    mUi->setupUi(widget);

    ObjectTypesModel *objectTypesModel = new ObjectTypesModel(this);
    objectTypesModel->setObjectTypes(Preferences::instance()->objectTypes());
    mUi->type->setModel(objectTypesModel);
    // No support for inserting new types at the moment
    mUi->type->setInsertPolicy(QComboBox::NoInsert);

    // Initialize UI with values from the map-object
    mUi->name->setText(mMapObject->name());
    mUi->type->setEditText(mMapObject->type());
    mUi->x->setValue(mMapObject->x());
    mUi->y->setValue(mMapObject->y());
    mUi->width->setValue(mMapObject->width());
    mUi->height->setValue(mMapObject->height());

    qobject_cast<QBoxLayout*>(layout())->insertWidget(0, widget);

    mUi->name->setFocus();

    // Resize the dialog to its recommended size
    resize(sizeHint());
}

ObjectPropertiesDialog::~ObjectPropertiesDialog()
{
    delete mUi;
}

void ObjectPropertiesDialog::accept()
{
    const QString newName = mUi->name->text();
    const QString newType = mUi->type->currentText();

    const qreal newPosX = mUi->x->value();
    const qreal newPosY = mUi->y->value();
    const qreal newWidth = mUi->width->value();
    const qreal newHeight = mUi->height->value();

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
    } else {
        PropertiesDialog::accept();
    }
}
