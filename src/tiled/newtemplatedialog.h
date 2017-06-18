/*
 * newtemplatedialog.h
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

#pragma once

#include <QDialog>
#include "objecttemplate.h"

namespace Ui {
class NewTemplateDialog;
}

namespace Tiled {

namespace Internal {

class NewTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewTemplateDialog(QList<QString> groupNames, QWidget *parent = 0);
    ~NewTemplateDialog();

    void createTemplate(QString &name, int &index);

private slots:
    void updateOkButton();

private:
    Ui::NewTemplateDialog *mUi;
    ObjectTemplate mNewObjectTemplate;
};

} // namespace Internal
} // namespace Tiled
