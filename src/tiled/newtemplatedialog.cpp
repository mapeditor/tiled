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
#include "ui_newtemplatedialog.h"

#include <QPushButton>

using namespace Tiled;
using namespace Tiled::Internal;

NewTemplateDialog::NewTemplateDialog(QList<QString> groupNames, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::NewTemplateDialog)
{
    mUi->setupUi(this);
    mUi->groupsComboBox->addItems(groupNames);
    connect(mUi->templateName, SIGNAL(textChanged(QString)), SLOT(updateOkButton()));
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
    QString templateName = mUi->templateName->text();

    int index = mUi->groupsComboBox->currentIndex();

    okButton->setEnabled(!templateName.isEmpty() && index != -1);
}
