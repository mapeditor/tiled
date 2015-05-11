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

#include "addpropertydialog.h"
#include "changeproperties.h"
#include "documentmanager.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "preferences.h"
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
    , mMapDocument(nullptr)
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

    mActionRenameProperty = new QAction(this);
    mActionRenameProperty->setEnabled(false);
    mActionRenameProperty->setIcon(QIcon(QLatin1String(":/images/16x16/rename.png")));
    connect(mActionRenameProperty, SIGNAL(triggered()),
            SLOT(renameProperty()));

    Utils::setThemeIcon(mActionAddProperty, "add");
    Utils::setThemeIcon(mActionRemoveProperty, "remove");
    Utils::setThemeIcon(mActionRenameProperty, "rename");

    QToolBar *toolBar = new QToolBar;
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setIconSize(QSize(16, 16));
    toolBar->addAction(mActionAddProperty);
    toolBar->addAction(mActionRemoveProperty);
    toolBar->addAction(mActionRenameProperty);

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

void PropertiesDock::setMapDocument(MapDocument *mapDocument)
{
    // Stop connecting to the DocumentManager singleton instance.
    DocumentManager *manager = DocumentManager::instance();
    disconnect(manager, SIGNAL(currentDocumentChanged(MapDocument*)), this,
            SLOT(mapDocumentChanged(MapDocument*)));

    // Connect to the document passed in.
    // Note: Since we are disconnected from the document manager now we are responsible for setting the map document from now on.
    mapDocumentChanged(mapDocument);
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
        currentObjectChanged(nullptr);
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

static bool isAddedByType(Object *object, const QString &name)
{
    if (!object || object->typeId() != Object::MapObjectType)
        return false;

    const QString objectType = static_cast<MapObject*>(object)->type();
    const ObjectTypes objectTypes = Preferences::instance()->objectTypes();
    for (const ObjectType &type : objectTypes) {
        if (type.name == objectType)
            return type.defaultProperties.contains(name);
    }

    return false;
}

void PropertiesDock::currentObjectChanged(Object *object)
{
    mPropertyBrowser->setObject(object);

    const bool external = isExternal(object);
    const bool isTileset = object && object->typeId() == Object::TilesetType;

    mPropertyBrowser->setEnabled(object && (!external || isTileset));
    mActionAddProperty->setEnabled(object && !external);
}

void PropertiesDock::currentItemChanged(QtBrowserItem *item)
{
    bool isCustomProperty = mPropertyBrowser->isCustomPropertyItem(item);
    bool external = isExternal(mPropertyBrowser->object());
    bool addedByType = item && isAddedByType(mPropertyBrowser->object(),
                                     item->property()->propertyName());
    bool canModify = isCustomProperty && !external && !addedByType;

    mActionRemoveProperty->setEnabled(canModify);
    mActionRenameProperty->setEnabled(canModify);
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
    AddPropertyDialog dialog(mPropertyBrowser);
    if (dialog.exec() == AddPropertyDialog::Accepted)
        addProperty(dialog.propertyName(), dialog.propertyType());
}

void PropertiesDock::addProperty(const QString &name, QVariant::Type type)
{
    if (name.isEmpty())
        return;
    Object *object = mMapDocument->currentObject();
    if (!object)
        return;

    if (!object->hasProperty(name)) {
        QUndoStack *undoStack = mMapDocument->undoStack();
        undoStack->push(new SetProperty(mMapDocument,
                                        mMapDocument->currentObjects(),
                                        name, QVariant(type)));
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
    QList<QtBrowserItem *> items = item->parent()->children();
    if (items.count() > 1) {
        int currentItemIndex = items.indexOf(item);
        if (item == items.last()) {
            mPropertyBrowser->setCurrentItem(items.at(currentItemIndex - 1));
        } else {
            mPropertyBrowser->setCurrentItem(items.at(currentItemIndex + 1));
        }
    }
    undoStack->push(new RemoveProperty(mMapDocument,
                                       mMapDocument->currentObjects(),
                                       name));
}

void PropertiesDock::renameProperty()
{
    QtBrowserItem *item = mPropertyBrowser->currentItem();
    if (!item)
        return;

    const QString oldName = item->property()->propertyName();

    QInputDialog *dialog = new QInputDialog(mPropertyBrowser);
    dialog->setInputMode(QInputDialog::TextInput);
    dialog->setLabelText(tr("Name:"));
    dialog->setTextValue(oldName);
    dialog->setWindowTitle(tr("Rename Property"));
    dialog->open(this, SLOT(renameProperty(QString)));
}

void PropertiesDock::renameProperty(const QString &name)
{
    if (name.isEmpty())
        return;

    QtBrowserItem *item = mPropertyBrowser->currentItem();
    if (!item)
        return;

    const QString oldName = item->property()->propertyName();
    if (oldName == name)
        return;

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->push(new RenameProperty(mMapDocument, mMapDocument->currentObjects(), oldName, name));
}


bool PropertiesDock::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::ShortcutOverride: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->matches(QKeySequence::Delete) || keyEvent->key() == Qt::Key_Backspace) {
            if (event->type() == QEvent::KeyPress)
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
    mActionRenameProperty->setText(tr("Rename Property"));
}

} // namespace Internal
} // namespace Tiled
