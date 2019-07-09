/*
 * fileedit.cpp
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

#include "fileedit.h"

#include <QFileDialog>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>

namespace Tiled {

FileEdit::FileEdit(QWidget *parent)
    : QWidget(parent)
    , mErrorTextColor(Qt::red)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    mLineEdit = new QLineEdit(this);
    mLineEdit->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));

    mOkTextColor = mLineEdit->palette().color(QPalette::Active, QPalette::Text);

    QToolButton *button = new QToolButton(this);
    button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    button->setFixedWidth(20);
    button->setText(QLatin1String("..."));
    layout->addWidget(mLineEdit);
    layout->addWidget(button);

    setFocusProxy(mLineEdit);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled);

    connect(mLineEdit, &QLineEdit::textEdited,
            this, &FileEdit::textEdited);
    connect(mLineEdit, &QLineEdit::textChanged,
            this, &FileEdit::validate);
    connect(button, &QAbstractButton::clicked,
            this, &FileEdit::buttonClicked);
}

void FileEdit::setFileUrl(const QUrl &url)
{
    const QString path = url.toString(QUrl::PreferLocalFile);
    if (mLineEdit->text() != path)
        mLineEdit->setText(path);
}

QUrl FileEdit::fileUrl() const
{
    const QString path = mLineEdit->text();
    QUrl url(path);
    if (url.isRelative())
        url = QUrl::fromLocalFile(path);
    return url;
}

void FileEdit::focusInEvent(QFocusEvent *e)
{
    mLineEdit->event(e);
    if (e->reason() == Qt::TabFocusReason || e->reason() == Qt::BacktabFocusReason) {
        mLineEdit->selectAll();
    }
    QWidget::focusInEvent(e);
}

void FileEdit::focusOutEvent(QFocusEvent *e)
{
    mLineEdit->event(e);
    QWidget::focusOutEvent(e);
}

void FileEdit::keyPressEvent(QKeyEvent *e)
{
    mLineEdit->event(e);
}

void FileEdit::keyReleaseEvent(QKeyEvent *e)
{
    mLineEdit->event(e);
}

void FileEdit::textEdited()
{
    emit fileUrlChanged(fileUrl());
}

void FileEdit::validate()
{
    const QUrl url(fileUrl());

    QColor textColor = mOkTextColor;
    if (url.isLocalFile() && !QFile::exists(url.toLocalFile()))
        textColor = mErrorTextColor;

    QPalette palette = mLineEdit->palette();
    palette.setColor(QPalette::Active, QPalette::Text, textColor);
    mLineEdit->setPalette(palette);
}

void FileEdit::buttonClicked()
{
    QUrl url = QFileDialog::getOpenFileUrl(window(),
                                           tr("Choose a File"),
                                           fileUrl(),
                                           mFilter);
    if (url.isEmpty())
        return;
    setFileUrl(url);
    emit fileUrlChanged(url);
}

} // namespace Tiled
