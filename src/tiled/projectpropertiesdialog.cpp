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

#include "mapformat.h"
#include "project.h"
#include "projectdocument.h"
#include "varianteditorfactory.h"
#include "variantpropertymanager.h"

#include <QtGroupPropertyManager>

namespace Tiled {

ProjectPropertiesDialog::ProjectPropertiesDialog(Project &project, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProjectPropertiesDialog)
    , mProject(project)
    , mPropertiesProjectDocument(new ProjectDocument(std::make_unique<Project>(), this))
{
    ui->setupUi(this);

    mPropertiesProjectDocument->project().setProperties(project.properties());

    auto variantPropertyManager = new VariantPropertyManager(this);
    auto variantEditorFactory = new VariantEditorFactory(this);
    auto groupPropertyManager = new QtGroupPropertyManager(this);

    ui->propertyBrowser->setFactoryForManager<QtVariantPropertyManager>(variantPropertyManager,
                                                                        variantEditorFactory);

    const QMap<CompatibilityVersion, QString> versionToName {
        { Tiled_1_8,             tr("Tiled 1.8") },
        { Tiled_1_9,             tr("Tiled 1.9") },
        { Tiled_1_10,            tr("Tiled 1.10") },
        { Tiled_Latest,          tr("Latest") },
    };
    mVersions = versionToName.keys();

    mCompatibilityVersionProperty = variantPropertyManager->addProperty(QtVariantPropertyManager::enumTypeId(),
                                                                        tr("Compatibility Version"));
    mCompatibilityVersionProperty->setAttribute(QLatin1String("enumNames"),
                                                QVariant::fromValue<QStringList>(versionToName.values()));
    mCompatibilityVersionProperty->setValue(mVersions.indexOf(project.mCompatibilityVersion));

    mExtensionPathProperty = variantPropertyManager->addProperty(filePathTypeId(), tr("Extensions Directory"));
    mExtensionPathProperty->setValue(project.mExtensionsPath);
    mExtensionPathProperty->setAttribute(QStringLiteral("directory"), true);

    QString ruleFileFilter = QCoreApplication::translate("File Types", "Automapping Rules files (*.txt)");
    FormatHelper<MapFormat> helper(FileFormat::ReadWrite, std::move(ruleFileFilter));

    mAutomappingRulesFileProperty = variantPropertyManager->addProperty(filePathTypeId(), tr("Automapping rules"));
    mAutomappingRulesFileProperty->setValue(project.mAutomappingRulesFile);
    mAutomappingRulesFileProperty->setAttribute(QStringLiteral("filter"), helper.filter());

    auto generalGroupProperty = groupPropertyManager->addProperty(tr("General"));
    generalGroupProperty->addSubProperty(mCompatibilityVersionProperty);

    auto filesGroupProperty = groupPropertyManager->addProperty(tr("Paths && Files"));
    filesGroupProperty->addSubProperty(mExtensionPathProperty);
    filesGroupProperty->addSubProperty(mAutomappingRulesFileProperty);

    ui->propertyBrowser->addProperty(generalGroupProperty);
    ui->propertyBrowser->addProperty(filesGroupProperty);

    ui->propertiesWidget->setDocument(mPropertiesProjectDocument);
}

ProjectPropertiesDialog::~ProjectPropertiesDialog()
{
    delete ui;
}

void ProjectPropertiesDialog::accept()
{
    mProject.setProperties(mPropertiesProjectDocument->project().properties());
    mProject.mCompatibilityVersion = mVersions.at(mCompatibilityVersionProperty->value().toInt());
    mProject.mExtensionsPath = mExtensionPathProperty->value().toString();
    mProject.mAutomappingRulesFile = mAutomappingRulesFileProperty->value().toString();

    QDialog::accept();
}

} // namespace Tiled

#include "moc_projectpropertiesdialog.cpp"
