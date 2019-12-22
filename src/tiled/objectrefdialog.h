/*
 * objectrefdialog.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

class ObjectGroup;
class QTableWidgetItem;

#include <QDialog>

namespace Ui {
class ObjectRefDialog;
}

namespace Tiled {

class MapObject;

class ObjectRefDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ObjectRefDialog(QWidget *parent = nullptr);
    ~ObjectRefDialog();

    void setId(const int id);
    int id() const;

private:
    void appendItem(const MapObject *object, QString objectPath);

    Ui::ObjectRefDialog *mUi;
    int mId;

private slots:
    void onTextChanged(const QString &text);
    void onItemSelectionChanged();
    void onItemDoubleClicked(QTableWidgetItem * item);
    void onButtonClicked(bool checked);
};

} // namespace Tiled
