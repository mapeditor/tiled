/*
 * projectpropertiesdialog.cpp
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "projectpropertiesdialog.h"
#include "ui_projectpropertiesdialog.h"

#include "project.h"
#include "utils.h"
#include "varianteditorfactory.h"
#include "variantpropertymanager.h"

#include <QtGroupPropertyManager>

namespace Tiled {

ProjectPropertiesDialog::ProjectPropertiesDialog(Project &project, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProjectPropertiesDialog)
    , mProject(project)
{
    ui->setupUi(this);

    auto variantPropertyManager = new VariantPropertyManager(this);
    auto variantEditorFactory = new VariantEditorFactory(this);
    auto groupPropertyManager = new QtGroupPropertyManager(this);

    ui->propertyBrowser->setFactoryForManager<QtVariantPropertyManager>(variantPropertyManager,
                                                                        variantEditorFactory);

    auto extensionsGroupProperty = groupPropertyManager->addProperty(tr("Extensions"));
    mExtensionPathProperty = variantPropertyManager->addProperty(filePathTypeId(), tr("Directory"));
    mExtensionPathProperty->setValue(project.mExtensionsPath);
    mExtensionPathProperty->setAttribute(QStringLiteral("directory"), true);
    extensionsGroupProperty->addSubProperty(mExtensionPathProperty);

    auto automappingGroupProperty = groupPropertyManager->addProperty(tr("Automapping"));
    mAutomappingRulesFileProperty = variantPropertyManager->addProperty(filePathTypeId(), tr("Rules file"));
    mAutomappingRulesFileProperty->setValue(project.mAutomappingRulesFile);
    mAutomappingRulesFileProperty->setAttribute(QStringLiteral("filter"), tr("Automapping Rules File (*.txt)"));
    automappingGroupProperty->addSubProperty(mAutomappingRulesFileProperty);

    ui->propertyBrowser->addProperty(extensionsGroupProperty);
    ui->propertyBrowser->addProperty(automappingGroupProperty);
}

ProjectPropertiesDialog::~ProjectPropertiesDialog()
{
    delete ui;
}

void ProjectPropertiesDialog::accept()
{
    mProject.mExtensionsPath = mExtensionPathProperty->value().toString();
    mProject.mAutomappingRulesFile = mAutomappingRulesFileProperty->value().toString();

    QDialog::accept();
}

} // namespace Tiled
