/*
 * custompropseditor.h
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>>
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

#include "properties.h"

#include <QDialog>

class QStringListModel;

namespace Ui {
class CustomTypesEditor;
}

namespace Tiled {

class CustomTypesModel;

class CustomTypesEditor : public QDialog
{
    Q_OBJECT

public:
    explicit CustomTypesEditor(QWidget *parent = nullptr);
    ~CustomTypesEditor() override;

signals:
    void closed();

protected:
    void closeEvent(QCloseEvent *) override;
    void changeEvent(QEvent *e) override;

private:
    void addCustomType();
    void selectedCustomTypesChanged();
    void removeSelectedCustomTypes();
    void customTypeIndexClicked(const QModelIndex &index);
    void applyCustomTypes();
    void customTypesChanged();

    void updateValues();
    void updateActions();

    void addValue();
    void removeValues();

    void selectFirstCustomType();
    void valuesChanged();

    void recalculateValues();
    void retranslateUi();

    void createValue(int row, const QString &name);

    Ui::CustomTypesEditor *mUi;
    CustomTypesModel *mCustomTypesModel;
    QStringListModel *mDetailsModel;

    bool mSettingPrefCustomTypes = false;
    bool mTouchingValues = false;

    QAction *mAddCustomTypeAction;
    QAction *mRemoveCustomTypeAction;

    QAction *mAddValueAction;
    QAction *mRemoveValueAction;
};

} // namespace Tiled
