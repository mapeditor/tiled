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

#include <QFormLayout>
#include <QGroupBox>

namespace Tiled {

WorldPropertiesDialog::WorldPropertiesDialog(WorldDocumentPtr world, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WorldPropertiesDialog)
    , mWorldDocument(world)
{
    ui->setupUi(this);

    const auto setGridSize = [=](int width, int height) {
        const QSize gridSize(width, height);
        if (gridSize == mWorldDocument->world()->gridSize)
            return;
        auto *undoStack = mWorldDocument->undoStack();
        undoStack->push(new SetWorldGridCommand(mWorldDocument.get(), gridSize));
    };

    mGridWidthProperty = new IntProperty(
                tr("Width"),
                [=] {
                    return mWorldDocument->world()->gridSize.width();
                },
                [=](const int &value) {
                    setGridSize(value, mWorldDocument->world()->gridSize.height());
                });
    mGridWidthProperty->setSuffix(tr(" px"));

    mGridHeightProperty = new IntProperty(
                tr("Height"),
                [=] {
                    return mWorldDocument->world()->gridSize.height();
                },
                [=](const int &value) {
                    setGridSize(mWorldDocument->world()->gridSize.width(), value);
                });
    mGridHeightProperty->setSuffix(tr(" px"));

    auto gridGroup = new QGroupBox(tr("World Grid"));
    auto gridLayout = new QFormLayout(gridGroup);
    gridLayout->addRow(mGridWidthProperty->name(), mGridWidthProperty->createEditor(gridGroup));
    gridLayout->addRow(mGridHeightProperty->name(), mGridHeightProperty->createEditor(gridGroup));

    ui->dialogLayout->insertWidget(0, gridGroup);

    // Don't display the "Custom Properties" header
    ui->propertiesWidget->customPropertiesGroup()->setName(QString());

    // Tweak margins
    const auto halfSpacing = Utils::dpiScaled(2);
    ui->propertiesWidget->propertiesView()->widget()->setContentsMargins(0, halfSpacing, 0, halfSpacing);

    ui->propertiesWidget->setDocument(mWorldDocument.get());

    // Keep the editors in sync if the world is changed elsewhere (e.g. undo).
    connect(mWorldDocument.get(), &WorldDocument::worldChanged,
            mGridWidthProperty, &Property::valueChanged);
    connect(mWorldDocument.get(), &WorldDocument::worldChanged,
            mGridHeightProperty, &Property::valueChanged);

    setWindowTitle(tr("%1 - World Properties").arg(world->displayName()));
}

WorldPropertiesDialog::~WorldPropertiesDialog()
{
    delete ui;
}

} // namespace Tiled

#include "moc_worldpropertiesdialog.cpp"
