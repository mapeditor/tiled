/*
 * tilesetparametersedit.cpp
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilesetparametersedit.h"

#include "newtilesetdialog.h"
#include "tileset.h"
#include "tilesetchanges.h"
#include "tilesetdocument.h"

#include <QFocusEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

namespace Tiled {

TilesetParametersEdit::TilesetParametersEdit(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    mLabel = new QLabel(this);
    mLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));

    QToolButton *button = new QToolButton(this);
    button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    button->setText(tr("Edit..."));
    layout->addWidget(mLabel);
    layout->addWidget(button);

    setFocusProxy(button);
    setFocusPolicy(Qt::StrongFocus);

    connect(button, &QToolButton::clicked,
            this, &TilesetParametersEdit::buttonClicked);
}

void TilesetParametersEdit::setTilesetDocument(TilesetDocument *tilesetDocument)
{
    mTilesetDocument = tilesetDocument;

    if (tilesetDocument)
        mLabel->setText(tilesetDocument->tileset()->imageSource().fileName());
    else
        mLabel->clear();
}

void TilesetParametersEdit::buttonClicked()
{
    if (!mTilesetDocument)
        return;

    TilesetParameters parameters(*mTilesetDocument->tileset());
    NewTilesetDialog dialog(window());

    if (dialog.editTilesetParameters(parameters)) {
        if (parameters != TilesetParameters(*mTilesetDocument->tileset())) {
            auto command = new ChangeTilesetParameters(mTilesetDocument,
                                                       parameters);

            mTilesetDocument->undoStack()->push(command);
        }
    }
}

} // namespace Tiled
