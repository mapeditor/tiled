/*
 * resizedialog.cpp
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Jeff Bland <jksb@member.fsf.org>
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

#include "resizedialog.h"
#include "ui_resizedialog.h"

using namespace Tiled::Internal;

ResizeDialog::ResizeDialog(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::ResizeDialog)
{
    mUi->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Initialize the new size of the resizeHelper to the default values of
    // the spin boxes. Otherwise, if the map width or height is default, then
    // setOldSize() will simply reset default values, causing callbacks in the
    // resize helper to not be called.
    mUi->resizeHelper->setNewSize(QSize(mUi->widthSpinBox->value(),
                                        mUi->heightSpinBox->value()));

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
