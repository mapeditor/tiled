/*
 * fileedit.h
 * Copyright (C) 2006 Trolltech ASA. All rights reserved. (GPLv2)
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef FILEEDIT_H
#define FILEEDIT_H

#include <QValidator>
#include <QWidget>

class QLineEdit;

namespace Tiled {
namespace Internal {

/**
 * A widget that combines a line edit with a button to choose a file.
 */
class FileEdit : public QWidget
{
    Q_OBJECT

public:
    explicit FileEdit(QWidget *parent = 0);

    void setFilePath(const QString &filePath);
    QString filePath() const;

    void setFilter(const QString &filter) { mFilter = filter; }
    QString filter() const { return mFilter; }

signals:
    void filePathChanged(const QString &filePath);

protected:
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

private slots:
    void validate(const QString &);
    void buttonClicked();

private:
    QLineEdit *mLineEdit;
    QString mFilter;
    QColor mOkTextColor;
    QColor mErrorTextColor;
};

} // namespace Internal
} // namespace Tiled

#endif // FILEEDIT_H
