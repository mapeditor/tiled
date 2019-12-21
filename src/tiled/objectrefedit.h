/*
 * objectrefedit.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * Based loosely on the TextPropertyEditor and TextEditor classes from
 * Qt Designer (Copyright (C) 2015 The Qt Company Ltd., LGPLv2.1).
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

#include <QWidget>

class QLineEdit;

namespace Tiled {

class ObjectRefEdit : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectRefEdit(QWidget *parent = nullptr);

    int id() const;

public slots:
    void setId(int id);

signals:
    void idChanged(int id);

private slots:
    void onIdChanged();
    void onButtonClicked();

private:
    QLineEdit *mLine;
    int mId;
};

} // namespace Tiled
