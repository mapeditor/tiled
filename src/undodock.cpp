/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
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

    QUndoView *undoView = new QUndoView(undoGroup, this);
    QIcon cleanIcon(QLatin1String(":images/16x16/drive-harddisk.png"));
    undoView->setCleanIcon(cleanIcon);
    undoView->setUniformItemSizes(true);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(5);
    layout->addWidget(undoView);

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
}
