/*
 * undodock.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Petr Viktorin <encukou@gmail.com>
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

#include "undodock.h"

#include <QEvent>
#include <QUndoView>
#include <QVBoxLayout>

using namespace Tiled;
using namespace Tiled::Internal;

UndoDock::UndoDock(QUndoGroup *undoGroup, QWidget *parent)
    : QDockWidget(parent)
{
    setObjectName(QLatin1String("undoViewDock"));

    mUndoView = new QUndoView(undoGroup, this);
    QIcon cleanIcon(QLatin1String(":images/16x16/drive-harddisk.png"));
    mUndoView->setCleanIcon(cleanIcon);
    mUndoView->setUniformItemSizes(true);
    mUndoView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(5);
    layout->addWidget(mUndoView);

    setWidget(widget);
    retranslateUi();
}

void UndoDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void UndoDock::retranslateUi()
{
    setWindowTitle(tr("History"));
    mUndoView->setEmptyLabel(tr("<empty>"));
}
