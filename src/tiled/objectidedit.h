/*
 * objectidedit.h
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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


#ifndef TILED_INTERNAL_OBJECTIDEDIT_H
#define TILED_INTERNAL_OBJECTIDEDIT_H

#include <QWidget>

class QSpinBox;

namespace Tiled {
namespace Internal {

class ObjectIdEdit : public QWidget
{
    Q_OBJECT

public:
    explicit ObjectIdEdit(QWidget *parent = nullptr);

    int id() const;

signals:

public slots:
    void setId(int id);

signals:
    void idChanged(int id);

private slots:
    void onIdChanged(int id);
    void onButtonClicked();

private:
    QSpinBox *mSpinBox;
    int mId;
};

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_OBJECTIDEDIT_H
