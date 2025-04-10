/*
 * WorldPropertiesDialog.cpp
 * Copyright 2025, Thorbjørn Lindeijer <bjorn@lindeijer.nl>, dogboydog
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
#include "propertiesview.h"
#include "ui_worldpropertiesdialog.h"

#include "utils.h"

#include <QFormLayout>
#include <QGroupBox>

namespace Tiled {

WorldPropertiesDialog::WorldPropertiesDialog(WorldDocumentPtr world, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WorldPropertiesDialog)
    , mLocalWorldDocument(world)
{
    ui->setupUi(this);

    ui->worldFilenameLabel->setText(world->fileName());
    // Don't display the "Custom Properties" header
    ui->propertiesWidget->customPropertiesGroup()->setName(QString());

    // Tweak margins
    const auto halfSpacing = Utils::dpiScaled(2);
    ui->propertiesWidget->propertiesView()->widget()->setContentsMargins(0, halfSpacing, 0, halfSpacing);

    ui->propertiesWidget->setDocument(mLocalWorldDocument.get());
}

WorldPropertiesDialog::~WorldPropertiesDialog()
{
    delete ui;
}

void WorldPropertiesDialog::accept()
{
    // do I need local vs shared world pointer ? if not , don't need to override this
    // mLocalWorldDocument->setProperties(world.properties());

    QDialog::accept();
}


} // namespace Tiled

#include "moc_worldpropertiesdialog.cpp"
