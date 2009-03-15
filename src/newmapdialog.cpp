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

#include "newmapdialog.h"
#include "ui_newmapdialog.h"

using namespace Tiled::Internal;

NewMapDialog::NewMapDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::NewMapDialog)
{
    mUi->setupUi(this);
}

NewMapDialog::~NewMapDialog()
{
    delete mUi;
}

int NewMapDialog::mapWidth() const
{
    return mUi->mapWidth->value();
}

int NewMapDialog::mapHeight() const
{
    return mUi->mapHeight->value();
}

int NewMapDialog::tileWidth() const
{
    return mUi->tileWidth->value();
}

int NewMapDialog::tileHeight() const
{
    return mUi->tileHeight->value();
}
