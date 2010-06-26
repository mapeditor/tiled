/*
 * propertiesdialog.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include "propertiesdialog.h"
#include "ui_propertiesdialog.h"

#include "changeproperties.h"
#include "propertiesmodel.h"

#include "objectgroup.h"
#include "objectgrouppropertiesdialog.h"

#include "mapdocument.h"

#include <QShortcut>
#include <QUndoStack>

using namespace Tiled::Internal;

PropertiesDialog::PropertiesDialog(const QString &kind,
                                   QMap<QString, QString> *properties,
                                   QUndoStack *undoStack,
                                   QWidget *parent):
    QDialog(parent),
    mUndoStack(undoStack),
    mProperties(properties),
    mKind(kind)
{
    mUi = new Ui::PropertiesDialog;
    mUi->setupUi(this);

    mModel = new PropertiesModel(this);
    mModel->setProperties(*mProperties);
    mUi->propertiesView->setModel(mModel);

    // Delete selected properties when the delete key is pressed
    QShortcut *deleteShortcut = new QShortcut(QKeySequence::Delete,
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

void PropertiesDialog::showDialogFor(Layer *layer,
                                     MapDocument *mapDocument,
                                     QWidget *parent)
{
    ObjectGroup *objectGroup = dynamic_cast<ObjectGroup*>(layer);
    PropertiesDialog *dialog;

    if (objectGroup) {
        dialog = new ObjectGroupPropertiesDialog(mapDocument,
                                                 objectGroup,
                                                 parent);
    } else {
        dialog = new PropertiesDialog(tr("Layer"),
                                      layer->properties(),
                                      mapDocument->undoStack(),
                                      parent);
    }

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
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
