/*
 * worldpropertiesdialog.cpp
 * Copyright 2025, dogboydog
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

#include "worldpropertiesdialog.h"
#include "ui_worldpropertiesdialog.h"

#include "changeworld.h"
#include "propertiesview.h"
#include "world.h"

#include "utils.h"

#include <QSignalBlocker>

namespace Tiled {

WorldPropertiesDialog::WorldPropertiesDialog(WorldDocumentPtr world, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WorldPropertiesDialog)
    , mWorldDocument(world)
{
    ui->setupUi(this);

    // Don't display the "Custom Properties" header
    ui->propertiesWidget->customPropertiesGroup()->setName(QString());

    // Tweak margins
    const auto halfSpacing = Utils::dpiScaled(2);
    ui->propertiesWidget->propertiesView()->widget()->setContentsMargins(0, halfSpacing, 0, halfSpacing);

    ui->propertiesWidget->setDocument(mWorldDocument.get());

    // Avoid one undo step per keystroke while typing a value.
    ui->gridWidthSpinBox->setKeyboardTracking(false);
    ui->gridHeightSpinBox->setKeyboardTracking(false);

    refreshGridFromWorld();

    connect(ui->gridWidthSpinBox, &QSpinBox::valueChanged,
            this, &WorldPropertiesDialog::pushGridSizeCommand);
    connect(ui->gridHeightSpinBox, &QSpinBox::valueChanged,
            this, &WorldPropertiesDialog::pushGridSizeCommand);

    // Keep the spin boxes in sync if the world is changed elsewhere (e.g. undo).
    connect(mWorldDocument.get(), &WorldDocument::worldChanged,
            this, &WorldPropertiesDialog::refreshGridFromWorld);

    setWindowTitle(tr("%1 - World Properties").arg(world->displayName()));
}

void WorldPropertiesDialog::pushGridSizeCommand()
{
    const QSize gridSize(ui->gridWidthSpinBox->value(),
                         ui->gridHeightSpinBox->value());

    if (gridSize == mWorldDocument->world()->gridSize)
        return;

    mWorldDocument->undoStack()->push(
                new SetWorldGridCommand(mWorldDocument.get(), gridSize));
}

void WorldPropertiesDialog::refreshGridFromWorld()
{
    const QSize gridSize = mWorldDocument->world()->gridSize;
    const QSignalBlocker blockWidth(ui->gridWidthSpinBox);
    const QSignalBlocker blockHeight(ui->gridHeightSpinBox);
    ui->gridWidthSpinBox->setValue(gridSize.width());
    ui->gridHeightSpinBox->setValue(gridSize.height());
}

WorldPropertiesDialog::~WorldPropertiesDialog()
{
    delete ui;
}

} // namespace Tiled

#include "moc_worldpropertiesdialog.cpp"
