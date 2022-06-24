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

#pragma once

#include <QValidator>
#include <QWidget>

class QLineEdit;

namespace Tiled {

/**
 * A widget that combines a line edit with a button to choose a file.
 */
class FileEdit : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString filter READ filter WRITE setFilter)
    Q_PROPERTY(QUrl fileUrl READ fileUrl WRITE setFileUrl)
    Q_PROPERTY(bool isDirectory READ isDirectory WRITE setIsDirectory)

public:
    explicit FileEdit(QWidget *parent = nullptr);

    void setFileUrl(const QUrl &url);
    QUrl fileUrl() const;

    void setFilter(const QString &filter) { mFilter = filter; }
    QString filter() const { return mFilter; }

    void setIsDirectory(bool isDirectory);
    bool isDirectory() const;

signals:
    void fileUrlChanged(const QUrl &url);

protected:
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

private:
    void textEdited();
    void validate();
    void buttonClicked();

    QLineEdit *mLineEdit;
    QString mFilter;
    bool mIsDirectory = false;
    QColor mOkTextColor;
    QColor mErrorTextColor;
};


inline void FileEdit::setIsDirectory(bool isDirectory)
{
    mIsDirectory = isDirectory;
}

inline bool FileEdit::isDirectory() const
{
    return mIsDirectory;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::FileEdit*)
