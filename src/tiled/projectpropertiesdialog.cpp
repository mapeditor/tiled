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
#include "propertiesview.h"
#include "tiled.h"
#include "utils.h"

#include <QFormLayout>
#include <QGroupBox>

namespace Tiled {

template<> EnumData enumData<CompatibilityVersion>()
{
    return {{
        QCoreApplication::translate("Tiled::ProjectPropertiesDialog", "Tiled 1.8"),
        QCoreApplication::translate("Tiled::ProjectPropertiesDialog", "Tiled 1.9"),
        QCoreApplication::translate("Tiled::ProjectPropertiesDialog", "Tiled 1.10"),
        QCoreApplication::translate("Tiled::ProjectPropertiesDialog", "Latest"),
    }, {
        Tiled_1_8,
        Tiled_1_9,
        Tiled_1_10,
        Tiled_Latest,
    }};
}

ProjectPropertiesDialog::ProjectPropertiesDialog(Project &project, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProjectPropertiesDialog)
    , mProject(project)
    , mLocalProjectDocument(new ProjectDocument(std::make_unique<Project>(project), this))
{
    ui->setupUi(this);

    mCompatibilityVersionProperty = new EnumProperty<CompatibilityVersion>(
                tr("Compatibility Version"),
                [=] {
                    return localProject().mCompatibilityVersion;
                },
                [=](CompatibilityVersion value) {
                    localProject().mCompatibilityVersion = value;
                });

    mExtensionPathProperty = new UrlProperty(
                tr("Extensions Directory"),
                [=] {
                    return QUrl::fromLocalFile(localProject().mExtensionsPath);
                },
                [=](const QUrl &value) {
                    localProject().mExtensionsPath = value.toLocalFile();
                });
    mExtensionPathProperty->setIsDirectory(true);

    QString ruleFileFilter = QCoreApplication::translate("File Types", "Automapping Rules files (*.txt)");
    FormatHelper<MapFormat> helper(FileFormat::ReadWrite, std::move(ruleFileFilter));

    mAutomappingRulesFileProperty = new UrlProperty(
                tr("Automapping rules"),
                [=] {
                    return QUrl::fromLocalFile(localProject().mAutomappingRulesFile);
                },
                [=](const QUrl &value) {
                    localProject().mAutomappingRulesFile = value.toLocalFile();
                });
    mAutomappingRulesFileProperty->setFilter(helper.filter());

    auto generalGroup = new QGroupBox(tr("General"));
    auto generalLayout = new QFormLayout(generalGroup);
    generalLayout->addRow(mCompatibilityVersionProperty->name(), mCompatibilityVersionProperty->createEditor(generalGroup));

    auto filesGroup = new QGroupBox(tr("Paths && Files"));
    auto filesLayout = new QFormLayout(filesGroup);
    filesLayout->addRow(mExtensionPathProperty->name(), mExtensionPathProperty->createEditor(filesGroup));
    filesLayout->addRow(mAutomappingRulesFileProperty->name(), mAutomappingRulesFileProperty->createEditor(filesGroup));

    ui->dialogLayout->insertWidget(0, filesGroup);
    ui->dialogLayout->insertWidget(0, generalGroup);

    // Don't display the "Custom Properties" header
    ui->propertiesWidget->customPropertiesGroup()->setName(QString());

    // Tweak margins
    const auto halfSpacing = Utils::dpiScaled(2);
    ui->propertiesWidget->propertiesView()->widget()->setContentsMargins(0, halfSpacing, 0, halfSpacing);

    ui->propertiesWidget->setDocument(mLocalProjectDocument);
}

ProjectPropertiesDialog::~ProjectPropertiesDialog()
{
    delete ui;
}

void ProjectPropertiesDialog::accept()
{
    auto &project = localProject();

    mProject.setProperties(project.properties());
    mProject.mCompatibilityVersion = project.mCompatibilityVersion;
    mProject.mExtensionsPath = project.mExtensionsPath;
    mProject.mAutomappingRulesFile = project.mAutomappingRulesFile;

    QDialog::accept();
}

Project &ProjectPropertiesDialog::localProject()
{
    return mLocalProjectDocument->project();
}

} // namespace Tiled

#include "moc_projectpropertiesdialog.cpp"
