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

namespace Tiled {
namespace Internal {

FileChangedWarning::FileChangedWarning(QWidget *parent)
    : QWidget(parent)
    , mLabel(new QLabel(this))
    , mButtons(new QDialogButtonBox(QDialogButtonBox::Yes |
                                    QDialogButtonBox::No,
                                    Qt::Horizontal,
                                    this))
{
    mLabel->setText(tr("File change detected. Discard changes and reload the map?"));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(mLabel);
    layout->addStretch(1);
    layout->addWidget(mButtons);
    setLayout(layout);

    connect(mButtons, SIGNAL(accepted()), SIGNAL(reload()));
    connect(mButtons, SIGNAL(rejected()), SIGNAL(ignore()));
}

} // namespace Internal
} // namespace Tiled
