/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
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

#include "propertiesdialog.h"
#include "ui_propertiesdialog.h"

#include "changeproperties.h"
#include "propertiesmodel.h"

#include <QShortcut>
#include <QUndoStack>

using namespace Tiled::Internal;

PropertiesDialog::PropertiesDialog(const QString &kind,
                                   QUndoStack *undoStack,
                                   QWidget *parent):
    QDialog(parent),
    mUndoStack(undoStack),
    mProperties(0),
    mKind(kind)
{
    mUi = new Ui::PropertiesDialog;
    mUi->setupUi(this);

    mModel = new PropertiesModel(this);
    mUi->propertiesView->setModel(mModel);

    QHeaderView *header = mUi->propertiesView->header();
    header->setResizeMode(QHeaderView::ResizeToContents);

    // Delete selected properties when the delete key is pressed
    QShortcut *deleteShortcut = new QShortcut(QKeySequence(Qt::Key_Delete),
                                              mUi->propertiesView);
    deleteShortcut->setContext(Qt::WidgetShortcut);
    connect(deleteShortcut, SIGNAL(activated()),
            this, SLOT(deleteSelectedProperties()));

    setWindowTitle(tr("%1 Properties").arg(mKind));
}

PropertiesDialog::~PropertiesDialog()
{
    delete mUi;
}

void PropertiesDialog::setProperties(QMap<QString, QString> *properties)
{
    mProperties = properties;
    mModel->setProperties(*properties);
}

void PropertiesDialog::accept()
{
    const QMap<QString, QString> &properties = mModel->properties();
    if (mProperties && *mProperties != properties) {
        mUndoStack->push(new ChangeProperties(mKind,
                                              mProperties,
                                              properties));
    }
    QDialog::accept();
}

void PropertiesDialog::deleteSelectedProperties()
{
    QItemSelectionModel *selection = mUi->propertiesView->selectionModel();
    const QModelIndexList indices = selection->selectedRows();
    if (!indices.isEmpty()) {
        mModel->deleteProperties(indices);
        selection->select(mUi->propertiesView->currentIndex(),
                          QItemSelectionModel::ClearAndSelect |
                          QItemSelectionModel::Rows);
    }
}
