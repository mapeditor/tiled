/*
 * propertiesdock.cpp
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

#include "propertiesdock.h"

#include "documentmanager.h"
#include "mapdocument.h"
#include "propertiesmodel.h"
#include "propertiesview.h"

#include <QEvent>
#include <QShortcut>
#include <QUndoStack>

namespace Tiled {
namespace Internal {

PropertiesDock::PropertiesDock(QWidget *parent)
    : QDockWidget(parent)
    , mMapDocument(0)
    , mPropertiesModel(new PropertiesModel(this))
    , mPropertiesView(new PropertiesView(this))
{
    setObjectName(QLatin1String("propertiesDock"));
    setWidget(mPropertiesView);

    mPropertiesView->setModel(mPropertiesModel);

    DocumentManager *manager = DocumentManager::instance();
    connect(manager, SIGNAL(currentDocumentChanged(MapDocument*)),
            SLOT(mapDocumentChanged(MapDocument*)));

    // Delete selected properties when the delete or backspace key is pressed
    QShortcut *deleteShortcut = new QShortcut(QKeySequence::Delete,
                                              mPropertiesView);
    QShortcut *deleteShortcutAlt =
            new QShortcut(QKeySequence(Qt::Key_Backspace),
                          mPropertiesView);
    deleteShortcut->setContext(Qt::WidgetShortcut);
    deleteShortcutAlt->setContext(Qt::WidgetShortcut);

    connect(deleteShortcut, SIGNAL(activated()),
            this, SLOT(deleteSelectedProperties()));
    connect(deleteShortcutAlt, SIGNAL(activated()),
            this, SLOT(deleteSelectedProperties()));

    retranslateUi();
}

void PropertiesDock::mapDocumentChanged(MapDocument *mapDocument)
{
    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mapDocument) {
        connect(mapDocument, SIGNAL(currentObjectChanged(Object*)),
                SLOT(currentObjectChanged(Object*)));

        currentObjectChanged(mapDocument->currentObject());
    }
}

void PropertiesDock::currentObjectChanged(Object *object)
{
    if (object)
        mPropertiesModel->setObject(mMapDocument, object);
    else
        mPropertiesModel->setObject(0, 0);

    mPropertiesView->setEnabled(object != 0);

    // TODO: If the current object is a tile, disable when this tile is from
    // an external tileset.
}

void PropertiesDock::changeEvent(QEvent *e)
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

void PropertiesDock::deleteSelectedProperties()
{
    QItemSelectionModel *selection = mPropertiesView->selectionModel();
    const QModelIndexList indices = selection->selectedRows();
    if (!indices.isEmpty()) {
        mPropertiesModel->deleteProperties(indices);
        selection->select(mPropertiesView->currentIndex(),
                          QItemSelectionModel::ClearAndSelect |
                          QItemSelectionModel::Rows);
    }
}

void PropertiesDock::retranslateUi()
{
    setWindowTitle(tr("Properties"));
}

} // namespace Internal
} // namespace Tiled
