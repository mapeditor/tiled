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
    , mButtons(new QDialogButtonBox(QDialogButtonBox::Yes |
                                    QDialogButtonBox::No,
                                    Qt::Horizontal,
                                    this))
{
    mLabel->setText(tr("File change detected. Discard changes and reload the file?"));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(mLabel);
    layout->addWidget(mButtons);
    layout->addStretch(1);
    setLayout(layout);

    mButtons->button(QDialogButtonBox::Yes)->setText(tr("Reload"));
    mButtons->button(QDialogButtonBox::No)->setText(tr("Ignore"));

    connect(mButtons, &QDialogButtonBox::accepted, this, &FileChangedWarning::reload);
    connect(mButtons, &QDialogButtonBox::rejected, this, &FileChangedWarning::ignore);
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
