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

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QUndoStack>
#include <QDoubleValidator>

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
    , mPosXEdit(new QLineEdit())
    , mPosYEdit(new QLineEdit())
{
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(new QLabel(tr("Name:")), 0, 0);
    grid->addWidget(new QLabel(tr("Type:")), 1, 0);
    grid->addWidget(mNameEdit, 0, 1, 1, 2 );
    grid->addWidget(mTypeEdit, 1, 1, 1, 2 );
    grid->addWidget(new QLabel(tr("Position:")), 2, 0);
    grid->addWidget(mPosXEdit, 2, 1);
    grid->addWidget(mPosYEdit, 2, 2);
    qobject_cast<QBoxLayout*>(layout())->insertLayout(0, grid);

    QDoubleValidator *xValidator = new QDoubleValidator(mPosXEdit);
    xValidator->setNotation(QDoubleValidator::StandardNotation);

    QDoubleValidator *yValidator = new QDoubleValidator(mPosYEdit);
    yValidator->setNotation(QDoubleValidator::StandardNotation);

    mPosXEdit->setValidator(xValidator);
    mPosYEdit->setValidator(yValidator);

    mPosXEdit->setText( QString::number( mMapObject->x() ) );
    mPosYEdit->setText( QString::number( mMapObject->y() ) );

    mNameEdit->setFocus();
}

void ObjectPropertiesDialog::accept()
{
    const QString newName = mNameEdit->text();
    const QString newType = mTypeEdit->text();

    bool conversionOK = false;
    qreal newPosX = mPosXEdit->text().toDouble( &conversionOK );

    if ( !conversionOK )
    {
        newPosX = mMapObject->x();
    }

    qreal newPosY = mPosYEdit->text().toDouble( &conversionOK );

    if ( !conversionOK )
    {
        newPosY = mMapObject->y();
    }

    bool changed = false;
    changed |= mMapObject->name() != newName;
    changed |= mMapObject->type() != newType;
    changed |= mMapObject->x() != newPosX;
    changed |= mMapObject->y() != newPosY;

    if (changed) {
        QUndoStack *undo = mMapDocument->undoStack();
        undo->beginMacro(tr("Change Object"));
        undo->push(new ChangeMapObject(mMapDocument, mMapObject,
                                       newName, newType));

        const QPointF oldPos = mMapObject->position();
        mMapObject->setX(newPosX);
        mMapObject->setY(newPosY);
        undo->push(new MoveMapObject(mMapDocument, mMapObject, oldPos));

        PropertiesDialog::accept(); // Let PropertiesDialog add its command
        undo->endMacro();

        mMapDocument->emitObjectChanged(mMapObject);
    } else {
        PropertiesDialog::accept();
    }
}
