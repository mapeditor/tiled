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
#include "clipboardmanager.h"
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
#include <QToolBar>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QMenu>

namespace Tiled {
namespace Internal {

PropertiesDock::PropertiesDock(QWidget *parent)
    : QDockWidget(parent)
    , mDocument(nullptr)
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
    mActionRemoveProperty->setShortcuts(QKeySequence::Delete);
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
    toolBar->setIconSize(Utils::smallIconSize());
    toolBar->addAction(mActionAddProperty);
    toolBar->addAction(mActionRemoveProperty);
    toolBar->addAction(mActionRenameProperty);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mPropertyBrowser);
    layout->addWidget(toolBar);
    widget->setLayout(layout);

    setWidget(widget);

    mPropertyBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mPropertyBrowser, &PropertyBrowser::customContextMenuRequested,
            this, &PropertiesDock::showContextMenu);
    connect(mPropertyBrowser, &PropertyBrowser::currentItemChanged,
            this, &PropertiesDock::updateActions);

    retranslateUi();
}

void PropertiesDock::setDocument(Document *document)
{
    if (mDocument)
        mDocument->disconnect(this);

    mDocument = document;
    mPropertyBrowser->setDocument(document);

    if (document) {
        connect(document, SIGNAL(currentObjectChanged(Object*)),
                SLOT(currentObjectChanged(Object*)));
        connect(document, SIGNAL(editCurrentObject()),
                SLOT(bringToFront()));

        connect(document, &Document::propertyAdded,
                this, &PropertiesDock::updateActions);
        connect(document, &Document::propertyRemoved,
                this, &PropertiesDock::updateActions);

        currentObjectChanged(document->currentObject());
    } else {
        currentObjectChanged(nullptr);
    }
}

void PropertiesDock::bringToFront()
{
    show();
    raise();
    mPropertyBrowser->setFocus();
}

static bool isPartOfTileset(const Object *object)
{
    if (!object)
        return false;

    switch (object->typeId()) {
    case Object::TilesetType:
    case Object::TileType:
    case Object::TerrainType:
        return true;
    default:
        return false;
    }
}

static bool anyObjectHasProperty(const QList<Object*> &objects, const QString &name)
{
    for (Object *obj : objects) {
        if (obj->hasProperty(name))
            return true;
    }
    return false;
}

void PropertiesDock::currentObjectChanged(Object *object)
{
    mPropertyBrowser->setObject(object);

    bool editingTileset = mDocument && mDocument->type() == Document::TilesetDocumentType;
    bool isTileset = isPartOfTileset(object);
    bool enabled = object && (!isTileset || editingTileset);

    mPropertyBrowser->setEnabled(enabled || isTileset);
    mActionAddProperty->setEnabled(enabled);
}

void PropertiesDock::updateActions()
{
    QtBrowserItem *item = mPropertyBrowser->currentItem();
    bool isCustomProperty = mPropertyBrowser->isCustomPropertyItem(item);
    bool editingTileset = mDocument && mDocument->type() == Document::TilesetDocumentType;
    bool isTileset = isPartOfTileset(mPropertyBrowser->object());
    bool canModify = isCustomProperty &&
            (!isTileset || editingTileset) &&
            anyObjectHasProperty(mDocument->currentObjects(), item->property()->propertyName());

    mActionRemoveProperty->setEnabled(canModify);
    mActionRenameProperty->setEnabled(canModify);
}

void PropertiesDock::cutProperty()
{
    copyProperty();
    removeProperty();
}

void PropertiesDock::copyProperty()
{
    QtBrowserItem *item = mPropertyBrowser->currentItem();
    if (!mPropertyBrowser->isCustomPropertyItem(item))
        return;

    Object *object = mPropertyBrowser->object();
    if (!object)
        return;

    const QString name = item->property()->propertyName();
    const QVariant value = object->property(name);

    Properties properties;
    properties.insert(name, value);

    ClipboardManager::instance()->setProperties(properties);
}

void PropertiesDock::pasteProperties()
{
    auto clipboardManager = ClipboardManager::instance();
    if (!clipboardManager->hasProperties())
        return;

    Object *object = mPropertyBrowser->object();
    if (!object)
        return;

    Properties properties = object->properties();
    properties.merge(clipboardManager->properties());

    if (object->properties() != properties) {
        QUndoStack *undoStack = mDocument->undoStack();
        undoStack->push(new ChangeProperties(mDocument, QString(), object,
                                             properties));
    }
}

void PropertiesDock::addProperty()
{
    AddPropertyDialog dialog(mPropertyBrowser);
    if (dialog.exec() == AddPropertyDialog::Accepted)
        addProperty(dialog.propertyName(), dialog.propertyValue());
}

void PropertiesDock::addProperty(const QString &name, const QVariant &value)
{
    if (name.isEmpty())
        return;
    Object *object = mDocument->currentObject();
    if (!object)
        return;

    if (!object->hasProperty(name)) {
        QUndoStack *undoStack = mDocument->undoStack();
        undoStack->push(new SetProperty(mDocument,
                                        mDocument->currentObjects(),
                                        name, value));
    }

    mPropertyBrowser->editCustomProperty(name);
}

void PropertiesDock::removeProperty()
{
    QtBrowserItem *item = mPropertyBrowser->currentItem();
    if (!mPropertyBrowser->isCustomPropertyItem(item))
        return;

    Object *object = mDocument->currentObject();
    if (!object)
        return;

    const QString name = item->property()->propertyName();
    QUndoStack *undoStack = mDocument->undoStack();
    undoStack->push(new RemoveProperty(mDocument,
                                       mDocument->currentObjects(),
                                       name));
}

void PropertiesDock::renameProperty()
{
    QtBrowserItem *item = mPropertyBrowser->currentItem();
    if (!mPropertyBrowser->isCustomPropertyItem(item))
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

    QUndoStack *undoStack = mDocument->undoStack();
    undoStack->push(new RenameProperty(mDocument, mDocument->currentObjects(), oldName, name));
}

void PropertiesDock::showContextMenu(const QPoint& pos)
{   
    const Object *object = mDocument->currentObject();
    if (!object)
        return;

    const QtBrowserItem *item = mPropertyBrowser->currentItem();
    const bool customPropertySelected = mPropertyBrowser->isCustomPropertyItem(item);

    QMenu contextMenu(mPropertyBrowser);
    QAction *cutAction = contextMenu.addAction(tr("Cu&t"));
    QAction *copyAction = contextMenu.addAction(tr("&Copy"));
    QAction *pasteAction = contextMenu.addAction(tr("&Paste"));
    contextMenu.addSeparator();
    QMenu *convertMenu = contextMenu.addMenu(tr("Convert To"));
    QAction *renameAction = contextMenu.addAction(tr("Rename..."));
    QAction *removeAction = contextMenu.addAction(tr("Remove"));

    cutAction->setShortcuts(QKeySequence::Cut);
    cutAction->setIcon(QIcon(QLatin1String(":/images/16x16/edit-cut.png")));
    cutAction->setEnabled(customPropertySelected);
    copyAction->setShortcuts(QKeySequence::Copy);
    copyAction->setIcon(QIcon(QLatin1String(":/images/16x16/edit-copy.png")));
    copyAction->setEnabled(customPropertySelected);
    pasteAction->setShortcuts(QKeySequence::Paste);
    pasteAction->setIcon(QIcon(QLatin1String(":/images/16x16/edit-paste.png")));
    pasteAction->setEnabled(ClipboardManager::instance()->hasProperties());
    renameAction->setEnabled(mActionRenameProperty->isEnabled());
    renameAction->setIcon(mActionRenameProperty->icon());
    removeAction->setEnabled(mActionRemoveProperty->isEnabled());
    removeAction->setShortcuts(mActionRemoveProperty->shortcuts());
    removeAction->setIcon(mActionRemoveProperty->icon());

    Utils::setThemeIcon(cutAction, "edit-cut");
    Utils::setThemeIcon(copyAction, "edit-copy");
    Utils::setThemeIcon(pasteAction, "edit-paste");
    Utils::setThemeIcon(removeAction, "remove");

    QString propertyName;

    if (customPropertySelected) {
        const int convertTo[] = {
            QVariant::Bool,
            QVariant::Color,
            QVariant::Double,
            filePathTypeId(),
            QVariant::Int,
            QVariant::String
        };

        propertyName = item->property()->propertyName();
        const QVariant propertyValue = object->property(propertyName);

        for (int toType : convertTo) {
            QVariant copy = propertyValue;
            if (propertyValue.userType() != toType && copy.convert(toType)) {
                QAction *action = convertMenu->addAction(typeToName(toType));
                action->setData(copy);
            }
        }
    }

    convertMenu->setEnabled(!convertMenu->actions().isEmpty());

    connect(cutAction, &QAction::triggered, this, &PropertiesDock::cutProperty);
    connect(copyAction, &QAction::triggered, this, &PropertiesDock::copyProperty);
    connect(pasteAction, &QAction::triggered, this, &PropertiesDock::pasteProperties);
    connect(renameAction, &QAction::triggered, this, static_cast<void (PropertiesDock::*)()>(&PropertiesDock::renameProperty));
    connect(removeAction, &QAction::triggered, this, &PropertiesDock::removeProperty);

    const QPoint globalPos = mPropertyBrowser->mapToGlobal(pos);
    const QAction *selectedItem = contextMenu.exec(globalPos);
    if (selectedItem && selectedItem->parentWidget() == convertMenu) {
        QUndoStack *undoStack = mDocument->undoStack();
        undoStack->push(new SetProperty(mDocument,
                                        mDocument->currentObjects(),
                                        propertyName, selectedItem->data()));
    }
}


bool PropertiesDock::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ShortcutOverride: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->matches(QKeySequence::Delete) || keyEvent->key() == Qt::Key_Backspace
                || keyEvent->matches(QKeySequence::Cut)
                || keyEvent->matches(QKeySequence::Copy)
                || keyEvent->matches(QKeySequence::Paste)) {
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

void PropertiesDock::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Delete) || event->key() == Qt::Key_Backspace) {
        removeProperty();
    } else if (event->matches(QKeySequence::Cut)) {
        cutProperty();
    } else if (event->matches(QKeySequence::Copy)) {
        copyProperty();
    } else if (event->matches(QKeySequence::Paste)) {
        pasteProperties();
    } else {
        QDockWidget::keyPressEvent(event);
    }
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
