/*
 * listedit.h
 * Copyright 2024, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

class QLabel;

namespace Tiled {

/**
 * The widget that enables the user to edit a list property.
 */
class ListEdit final : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QVariantList value READ value WRITE setValue NOTIFY valueChanged FINAL)

public:
    explicit ListEdit(QWidget *parent = nullptr);

    const QVariantList &value() const { return mValue; }
    void setValue(const QVariantList &value);

    static QString valueText(const QVariantList &value);

signals:
    void valueChanged(const QVariantList &value);

private:
    void addButtonClicked();
    void editButtonClicked();

    QLabel *mLabel;
    QVariantList mValue;
};

} // namespace Tiled
