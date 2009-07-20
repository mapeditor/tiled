/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
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

#include "resizedialog.h"
#include "ui_resizedialog.h"

using namespace Tiled::Internal;

ResizeDialog::ResizeDialog(QWidget *parent)
    : QDialog(parent)
{
    mUi = new Ui::ResizeDialog;
    mUi->setupUi(this);

    connect(mUi->resizeHelper, SIGNAL(offsetBoundsChanged(QRect)),
                               SLOT(updateOffsetBounds(QRect)));
}

ResizeDialog::~ResizeDialog()
{
    delete mUi;
}

void ResizeDialog::setOldSize(const QSize &size)
{
    mUi->resizeHelper->setOldSize(size);

    // Reset the spin boxes to the old size
    mUi->widthSpinBox->setValue(size.width());
    mUi->heightSpinBox->setValue(size.height());
}

const QSize &ResizeDialog::newSize() const
{
    return mUi->resizeHelper->newSize();
}

const QPoint &ResizeDialog::offset() const
{
    return mUi->resizeHelper->offset();
}

void ResizeDialog::updateOffsetBounds(const QRect &bounds)
{
    mUi->offsetXSpinBox->setRange(bounds.left(), bounds.right());
    mUi->offsetYSpinBox->setRange(bounds.top(), bounds.bottom());
}
