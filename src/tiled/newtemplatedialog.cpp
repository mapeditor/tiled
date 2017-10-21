/*
 * newtemplatedialog.cpp
 * Copyright 2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Mohamed Thabet <thabetx@gmail.com>
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

#include "newtemplatedialog.h"
#include "objecttemplatemodel.h"
#include "preferences.h"
#include "templatemanager.h"
#include "tmxmapformat.h"
#include "ui_newtemplatedialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

using namespace Tiled;
using namespace Tiled::Internal;

NewTemplateDialog::NewTemplateDialog(const QString &objectName, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::NewTemplateDialog)
{
    mUi->setupUi(this);

    mUi->templateName->setText(objectName);

    connect(mUi->createGroupButton, &QPushButton::pressed,
            this, &NewTemplateDialog::createGroup);

    auto model = ObjectTemplateModel::instance();
    mUi->groupsComboBox->setModel(model);

    connect(mUi->templateName, &QLineEdit::textChanged,
            this, &NewTemplateDialog::updateOkButton);

    updateOkButton();
}

NewTemplateDialog::~NewTemplateDialog()
{
    delete mUi;
}

void NewTemplateDialog::createTemplate(QString &name, int &index)
{
    if (exec() != QDialog::Accepted)
        return;

    name = mUi->templateName->text();
    index = mUi->groupsComboBox->currentIndex();
    accept();
}

void NewTemplateDialog::updateOkButton()
{
    QPushButton *okButton = mUi->buttonBox->button(QDialogButtonBox::Ok);

    bool noTemplateName = mUi->templateName->text().isEmpty();
    int index = mUi->groupsComboBox->currentIndex();

    okButton->setDisabled(noTemplateName || index == -1);
}

void NewTemplateDialog::createGroup()
{
    newTemplateGroup();
    mUi->groupsComboBox->setCurrentIndex(mUi->groupsComboBox->count() - 1);
    updateOkButton();
}

void NewTemplateDialog::newTemplateGroup()
{
    FormatHelper<TemplateGroupFormat> helper(FileFormat::ReadWrite);
    QString filter = helper.filter();
    QString selectedFilter = TgxTemplateGroupFormat().nameFilter();

    Preferences *prefs = Preferences::instance();
    QString suggestedFileName = prefs->lastPath(Preferences::TemplateDocumentsFile);
    suggestedFileName += tr("/untitled.tgx");

    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save File"),
                                                    suggestedFileName,
                                                    filter,
                                                    &selectedFilter);

    if (fileName.isEmpty())
        return;

    auto templateGroup = new TemplateGroup();
    templateGroup->setName(QFileInfo(fileName).baseName());
    templateGroup->setFileName(fileName);
    QScopedPointer<TemplateGroupDocument>
        templateGroupDocument(new TemplateGroupDocument(templateGroup));

    TemplateGroupFormat *format = helper.formatByNameFilter(selectedFilter);
    templateGroup->setFormat(format);

    QString error;
    if (!templateGroupDocument->save(fileName, &error)) {
        QMessageBox::critical(nullptr, tr("Error Creating Template Group"), error);
        return;
    }

    auto model = ObjectTemplateModel::instance();
    model->addNewDocument(templateGroupDocument.take());

    prefs->setLastPath(Preferences::TemplateDocumentsFile,
                       QFileInfo(fileName).path());
}
