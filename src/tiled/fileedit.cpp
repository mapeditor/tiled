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
namespace Internal {

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

    connect(mLineEdit, SIGNAL(textEdited(QString)),
            this, SIGNAL(filePathChanged(QString)));
    connect(mLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(validate(QString)));
    connect(button, SIGNAL(clicked()),
            this, SLOT(buttonClicked()));
}

void FileEdit::setFilePath(const QString &filePath)
{
     if (mLineEdit->text() != filePath)
         mLineEdit->setText(filePath);
}

QString FileEdit::filePath() const
{
    return mLineEdit->text();
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

void FileEdit::validate(const QString &text)
{
    QColor textColor = QFile::exists(text) ? mOkTextColor : mErrorTextColor;

    QPalette palette = mLineEdit->palette();
    palette.setColor(QPalette::Active, QPalette::Text, textColor);
    mLineEdit->setPalette(palette);
}

void FileEdit::buttonClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Choose a File"), mLineEdit->text(), mFilter);
    if (filePath.isNull())
        return;
    mLineEdit->setText(filePath);
    emit filePathChanged(filePath);
}

} // namespace Internal
} // namespace Tiled
