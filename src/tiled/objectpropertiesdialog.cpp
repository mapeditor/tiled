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
#include "rotatemapobject.h"

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
                       mapDocument,
                       parent)
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
    mUi->rotation->setValue(mMapObject->rotation());

    qobject_cast<QBoxLayout*>(layout())->insertWidget(0, widget);

    mUi->name->setFocus();

    connect(mUi->name, SIGNAL(textEdited(QString)),
            SLOT(nameOrTypeEdited()));
    connect(mUi->type, SIGNAL(editTextChanged(QString)),
            SLOT(nameOrTypeEdited()));
    connect(mUi->x, SIGNAL(valueChanged(double)),
            SLOT(positionEdited()));
    connect(mUi->y, SIGNAL(valueChanged(double)),
            SLOT(positionEdited()));
    connect(mUi->width, SIGNAL(valueChanged(double)),
            SLOT(sizeEdited()));
    connect(mUi->height, SIGNAL(valueChanged(double)),
            SLOT(sizeEdited()));
    connect(mUi->rotation, SIGNAL(valueChanged(double)),
            SLOT(rotationEdited()));

    connect(mapDocument, SIGNAL(objectsChanged(QList<MapObject*>)),
            SLOT(objectsChanged(QList<MapObject*>)));
}

ObjectPropertiesDialog::~ObjectPropertiesDialog()
{
    delete mUi;
}

void ObjectPropertiesDialog::nameOrTypeEdited()
{
    const QString newName = mUi->name->text();
    const QString newType = mUi->type->currentText();

    if (mMapObject->name() == newName && mMapObject->type() == newType)
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->push(new ChangeMapObject(mapDocument(), mMapObject,
                                        newName, newType));
}

void ObjectPropertiesDialog::positionEdited()
{
    const QPointF newPos(mUi->x->value(),
                         mUi->y->value());

    if (mMapObject->position() == newPos)
        return;

    const QPointF oldPos = mMapObject->position();
    mMapObject->setPosition(newPos);

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->push(new MoveMapObject(mapDocument(), mMapObject, oldPos));
}

void ObjectPropertiesDialog::sizeEdited()
{
    const QSizeF newSize(mUi->width->value(),
                         mUi->height->value());

    if (mMapObject->size() == newSize)
        return;

    const QSizeF oldSize = mMapObject->size();
    mMapObject->setSize(newSize);

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->push(new ResizeMapObject(mapDocument(), mMapObject, oldSize));
}

void ObjectPropertiesDialog::rotationEdited()
{
    const qreal newRotation = mUi->rotation->value();
    if (mMapObject->rotation() == newRotation)
        return;

    const qreal oldRotation = mMapObject->rotation();
    mMapObject->setRotation(newRotation);

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->push(new RotateMapObject(mapDocument(), mMapObject, oldRotation));
}

void ObjectPropertiesDialog::objectsChanged(const QList<MapObject *> &objects)
{
    foreach (MapObject *object, objects) {
        if (object != mMapObject)
            continue;

        if (mUi->name->text() != object->name())
            mUi->name->setText(object->name());

        if (mUi->type->currentText() != object->type())
            mUi->type->setEditText(object->type());

        mUi->x->setValue(object->x());
        mUi->y->setValue(object->y());
        mUi->width->setValue(object->width());
        mUi->height->setValue(object->height());
        mUi->rotation->setValue(object->rotation());
        break;
    }
}
