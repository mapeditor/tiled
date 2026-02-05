/*
 * filechangedwarning.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindijer.nl>
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

#include "filechangedwarning.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>

namespace Tiled {

FileChangedWarning::FileChangedWarning(QWidget *parent)
    : QWidget(parent)
    , mLabel(new QLabel(this))
    , mButtons(new QDialogButtonBox(Qt::Horizontal, this))
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(mLabel);
    layout->addWidget(mButtons);
    layout->addStretch(1);
    setLayout(layout);

    setState(FileChanged);
}

void FileChangedWarning::setState(State state)
{
    setupButtonsForState(state);

    switch (state) {
    case FileChanged:
        mLabel->setText(tr("File change detected. Discard changes and reload the file?"));
        break;
    case FileDeleted:
        mLabel->setText(tr("This file was deleted on disk."));
        break;
    case FileRecreated:
        mLabel->setText(tr("File was recreated on disk."));
        break;
    }
}

void FileChangedWarning::setupButtonsForState(State state)
{
    const QList<QAbstractButton*> buttons = mButtons->buttons();
    for (QAbstractButton *button : buttons) {
        mButtons->removeButton(button);
        delete button;
    }

    switch (state) {
    case FileChanged: {
        QPushButton *reloadButton = mButtons->addButton(tr("Reload"), QDialogButtonBox::AcceptRole);
        QPushButton *ignoreButton = mButtons->addButton(tr("Ignore"), QDialogButtonBox::RejectRole);
        connect(reloadButton, &QPushButton::clicked, this, &FileChangedWarning::reload);
        connect(ignoreButton, &QPushButton::clicked, this, &FileChangedWarning::ignore);
        break;
    }
    case FileDeleted: {
        QPushButton *restoreButton = mButtons->addButton(tr("Restore"), QDialogButtonBox::AcceptRole);
        QPushButton *saveAsButton = mButtons->addButton(tr("Save As..."), QDialogButtonBox::ActionRole);
        QPushButton *closeButton = mButtons->addButton(tr("Close"), QDialogButtonBox::RejectRole);
        connect(restoreButton, &QPushButton::clicked, this, &FileChangedWarning::restore);
        connect(saveAsButton, &QPushButton::clicked, this, &FileChangedWarning::saveAs);
        connect(closeButton, &QPushButton::clicked, this, &FileChangedWarning::closeDocument);
        break;
    }
    case FileRecreated: {
        QPushButton *reloadButton = mButtons->addButton(tr("Reload Disk Version"), QDialogButtonBox::AcceptRole);
        QPushButton *keepButton = mButtons->addButton(tr("Override"), QDialogButtonBox::RejectRole);
        connect(reloadButton, &QPushButton::clicked, this, &FileChangedWarning::reload);
        connect(keepButton, &QPushButton::clicked, this, &FileChangedWarning::ignore);
        break;
    }
    }
}

void FileChangedWarning::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    const QPalette p = palette();
    const QRect r = rect();
    const QColor light = p.midlight().color();
    const QColor shadow = p.mid().color();

    QPainter painter(this);
    painter.setPen(light);
    painter.drawLine(r.bottomLeft(), r.bottomRight());
    painter.setPen(shadow);
    painter.drawLine(r.left(), r.bottom() - 1,
                     r.right(), r.bottom() - 1);

}

} // namespace Tiled

#include "moc_filechangedwarning.cpp"
