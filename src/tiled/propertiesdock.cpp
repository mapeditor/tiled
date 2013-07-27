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

#include "changeproperties.h"
#include "documentmanager.h"
#include "mapdocument.h"
#include "propertybrowser.h"
#include "terrain.h"
#include "tile.h"
#include "tileset.h"
#include "utils.h"

#include <QAction>
#include <QEvent>
#include <QInputDialog>
#include <QKeyEvent>
#include <QShortcut>
#include <QToolBar>
#include <QUndoStack>
#include <QVBoxLayout>

namespace Tiled {
namespace Internal {

PropertiesDock::PropertiesDock(QWidget *parent)
    : QDockWidget(parent)
    , mMapDocument(0)
    , mPropertyBrowser(new PropertyBrowser)
{
    setObjectName(QLatin1String("propertiesDock"));

    mActionAddProperty = new QAction(this);
    mActionAddProperty->setEnabled(false);
    mActionAddProperty->setIcon(QIcon(QLatin1String(":/images/16x16/add.png")));
    connect(mActionAddProperty, SIGNAL(triggered()),
            SLOT(addProperty()));

    mActionRemoveProperty = new QAction(this);
    mActionRemoveProperty->setEnabled(false);
    mActionRemoveProperty->setIcon(QIcon(QLatin1String(":/images/16x16/remove.png")));
    connect(mActionRemoveProperty, SIGNAL(triggered()),
            SLOT(removeProperty()));

    Utils::setThemeIcon(mActionAddProperty, "add");
    Utils::setThemeIcon(mActionRemoveProperty, "remove");

    QToolBar *toolBar = new QToolBar;
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setIconSize(QSize(16, 16));
    toolBar->addAction(mActionAddProperty);
    toolBar->addAction(mActionRemoveProperty);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(5);
    layout->setSpacing(0);
    layout->addWidget(mPropertyBrowser);
    layout->addWidget(toolBar);
    widget->setLayout(layout);

    setWidget(widget);

    DocumentManager *manager = DocumentManager::instance();
    connect(manager, SIGNAL(currentDocumentChanged(MapDocument*)),
            SLOT(mapDocumentChanged(MapDocument*)));

    connect(mPropertyBrowser, SIGNAL(currentItemChanged(QtBrowserItem*)),
            SLOT(currentItemChanged(QtBrowserItem*)));

    retranslateUi();
}

void PropertiesDock::bringToFront()
{
    show();
    raise();
    mPropertyBrowser->setFocus();
}

void PropertiesDock::mapDocumentChanged(MapDocument *mapDocument)
{
    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;
    mPropertyBrowser->setMapDocument(mapDocument);

    if (mapDocument) {
        connect(mapDocument, SIGNAL(currentObjectChanged(Object*)),
                SLOT(currentObjectChanged(Object*)));
        connect(mapDocument, SIGNAL(tilesetFileNameChanged(Tileset*)),
                SLOT(tilesetFileNameChanged(Tileset*)));
        connect(mapDocument, SIGNAL(editCurrentObject()),
                SLOT(bringToFront()));

        currentObjectChanged(mapDocument->currentObject());
    } else {
        currentObjectChanged(0);
    }
}

static bool isExternal(const Object *object)
{
    if (!object)
        return false;

    switch (object->typeId()) {
    case Object::TilesetType:
        return static_cast<const Tileset*>(object)->isExternal();
    case Object::TileType:
        return static_cast<const Tile*>(object)->tileset()->isExternal();
    case Object::TerrainType:
        return static_cast<const Terrain*>(object)->tileset()->isExternal();
    default:
        return false;
    }
}

void PropertiesDock::currentObjectChanged(Object *object)
{
    mPropertyBrowser->setObject(object);

    const bool enabled = object != 0 && !isExternal(object);
    mPropertyBrowser->setEnabled(enabled);
    mActionAddProperty->setEnabled(enabled);
}

void PropertiesDock::currentItemChanged(QtBrowserItem *item)
{
    bool isCustomProperty = mPropertyBrowser->isCustomPropertyItem(item);
    bool external = isExternal(mPropertyBrowser->object());
    mActionRemoveProperty->setEnabled(isCustomProperty && !external);
}

void PropertiesDock::tilesetFileNameChanged(Tileset *tileset)
{
    Object *object = mMapDocument->currentObject();
    if (!object)
        return;

    bool update = false;

    switch (object->typeId()) {
    case Object::TilesetType:
        update = object == tileset;
        break;
    case Object::TileType:
        update = static_cast<Tile*>(object)->tileset() == tileset;
        break;
    case Object::TerrainType:
        update = static_cast<Terrain*>(object)->tileset() == tileset;
        break;
    default:
        break;
    }

    if (update) {
        currentObjectChanged(object);
        currentItemChanged(mPropertyBrowser->currentItem());
    }
}

void PropertiesDock::addProperty()
{
    QInputDialog *dialog = new QInputDialog(mPropertyBrowser);
    dialog->setInputMode(QInputDialog::TextInput);
    dialog->setLabelText(tr("Name:"));
    dialog->setWindowTitle(tr("Add Property"));
    dialog->open(this, SLOT(addProperty(QString)));
}

void PropertiesDock::addProperty(const QString &name)
{
    if (name.isEmpty())
        return;
    Object *object = mMapDocument->currentObject();
    if (!object)
        return;

    if (!object->hasProperty(name)) {
        QUndoStack *undoStack = mMapDocument->undoStack();
        undoStack->push(new SetProperty(mMapDocument, object, name, QString()));
    }

    mPropertyBrowser->editCustomProperty(name);
}

void PropertiesDock::removeProperty()
{
    QtBrowserItem *item = mPropertyBrowser->currentItem();
    Object *object = mMapDocument->currentObject();
    if (!item || !object)
        return;

    const QString name = item->property()->propertyName();
    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->push(new RemoveProperty(mMapDocument, object, name));

    // TODO: Would be nice to automatically select the next property
}

bool PropertiesDock::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::ShortcutOverride: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->matches(QKeySequence::Delete) || keyEvent->key() == Qt::Key_Backspace) {
            removeProperty();
            event->accept();
            return true;
        }
        break;
    }
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }

    return QDockWidget::event(event);
}

void PropertiesDock::retranslateUi()
{
    setWindowTitle(tr("Properties"));

    mActionAddProperty->setText(tr("Add Property"));
    mActionRemoveProperty->setText(tr("Remove Property"));
}

} // namespace Internal
} // namespace Tiled
