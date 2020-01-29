/*
 * textpropertyedit.h
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

#pragma once

#include <QWidget>

class QLineEdit;

namespace Tiled {

QString escapeNewlines(const QString &string);

/**
 * An inline text editor used by the property browser. Implements option of
 * opening a multi-line text editor dialog.
 */
class TextPropertyEdit : public QWidget
{
    Q_OBJECT

public:
    explicit TextPropertyEdit(QWidget *parent = nullptr);

    QString text() const;

public slots:
    void setText(const QString &text);

signals:
    void textChanged(const QString &text);

private:
    void onTextChanged(const QString &text);
    void onButtonClicked();

    QLineEdit *mLineEdit;
    QString mCachedText;
};

} // namespace Tiled
