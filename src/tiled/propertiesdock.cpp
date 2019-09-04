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

PropertiesDock::PropertiesDock(QWidget *parent)
    : QDockWidget(parent)
    , mDocument(nullptr)
    , mPropertyBrowser(new PropertyBrowser)
{
    setObjectName(QLatin1String("propertiesDock"));

    mActionAddProperty = new QAction(this);
    mActionAddProperty->setEnabled(false);
    mActionAddProperty->setIcon(QIcon(QLatin1String(":/images/16/add.png")));
    connect(mActionAddProperty, &QAction::triggered,
            this, &PropertiesDock::openAddPropertyDialog);

    mActionRemoveProperty = new QAction(this);
    mActionRemoveProperty->setEnabled(false);
    mActionRemoveProperty->setIcon(QIcon(QLatin1String(":/images/16/remove.png")));
    mActionRemoveProperty->setShortcuts(QKeySequence::Delete);
    connect(mActionRemoveProperty, &QAction::triggered,
            this, &PropertiesDock::removeProperties);

    mActionRenameProperty = new QAction(this);
    mActionRenameProperty->setEnabled(false);
    mActionRenameProperty->setIcon(QIcon(QLatin1String(":/images/16/rename.png")));
    connect(mActionRenameProperty, &QAction::triggered,
            this, &PropertiesDock::renameProperty);

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
    connect(mPropertyBrowser, &PropertyBrowser::selectedItemsChanged,
            this, &PropertiesDock::updateActions);

    retranslateUi();
}

void PropertiesDock::setDocument(Document *document)
{
    if (mDocument == document)
        return;

    if (mDocument)
        mDocument->disconnect(this);

    mDocument = document;
    mPropertyBrowser->setDocument(document);

    if (document) {
        connect(document, &Document::currentObjectChanged,
                this, &PropertiesDock::currentObjectChanged);
        connect(document, &Document::editCurrentObject,
                this, &PropertiesDock::bringToFront);

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

void PropertiesDock::selectCustomProperty(const QString &name)
{
    bringToFront();
    mPropertyBrowser->selectCustomProperty(name);
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
    bool isTileset = object && object->isPartOfTileset();
    bool enabled = object && (!isTileset || editingTileset);

    mPropertyBrowser->setEnabled(object);
    mActionAddProperty->setEnabled(enabled);
}

void PropertiesDock::updateActions()
{
    const QList<QtBrowserItem*> items = mPropertyBrowser->selectedItems();
    bool allCustomProperties = !items.isEmpty() && mPropertyBrowser->allCustomPropertyItems(items);
    bool editingTileset = mDocument && mDocument->type() == Document::TilesetDocumentType;
    bool isTileset = mPropertyBrowser->object() && mPropertyBrowser->object()->isPartOfTileset();
    bool canModify = allCustomProperties && (!isTileset || editingTileset);

    // Disable remove and rename actions when none of the selected objects
    // actually have the selected property (it may be inherited).
    if (canModify) {
        for (QtBrowserItem *item : items) {
            if (!anyObjectHasProperty(mDocument->currentObjects(), item->property()->propertyName())) {
                canModify = false;
                break;
            }
        }
    }

    mActionRemoveProperty->setEnabled(canModify);
    mActionRenameProperty->setEnabled(canModify && items.size() == 1);
}

void PropertiesDock::cutProperties()
{
    if (copyProperties())
        removeProperties();
}

bool PropertiesDock::copyProperties()
{
    Object *object = mPropertyBrowser->object();
    if (!object)
        return false;

    Properties properties;

    const QList<QtBrowserItem*> items = mPropertyBrowser->selectedItems();
    for (QtBrowserItem *item : items) {
        if (!mPropertyBrowser->isCustomPropertyItem(item))
            return false;

        const QString name = item->property()->propertyName();
        const QVariant value = object->property(name);
        if (!value.isValid())
            return false;

        properties.insert(name, value);
    }

    ClipboardManager::instance()->setProperties(properties);
    return true;
}

void PropertiesDock::pasteProperties()
{
    auto clipboardManager = ClipboardManager::instance();

    Properties pastedProperties = clipboardManager->properties();
    if (pastedProperties.isEmpty())
        return;

    const QList<Object *> objects = mDocument->currentObjects();
    if (objects.isEmpty())
        return;

    QList<QUndoCommand*> commands;

    for (Object *object : objects) {
        Properties properties = object->properties();
        properties.merge(pastedProperties);

        if (object->properties() != properties) {
            commands.append(new ChangeProperties(mDocument, QString(), object,
                                                 properties));
        }
    }

    if (!commands.isEmpty()) {
        QUndoStack *undoStack = mDocument->undoStack();
        undoStack->beginMacro(tr("Paste Property/Properties", nullptr,
                                 pastedProperties.size()));

        for (QUndoCommand *command : commands)
            undoStack->push(command);

        undoStack->endMacro();
    }
}

void PropertiesDock::openAddPropertyDialog()
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

void PropertiesDock::removeProperties()
{
    Object *object = mDocument->currentObject();
    if (!object)
        return;

    const QList<QtBrowserItem*> items = mPropertyBrowser->selectedItems();
    if (items.isEmpty() || !mPropertyBrowser->allCustomPropertyItems(items))
        return;

    QStringList propertyNames;
    for (QtBrowserItem *item : items)
        propertyNames.append(item->property()->propertyName());

    QUndoStack *undoStack = mDocument->undoStack();
    undoStack->beginMacro(tr("Remove Property/Properties", nullptr,
                             propertyNames.size()));

    for (const QString &name : propertyNames) {
        undoStack->push(new RemoveProperty(mDocument,
                                           mDocument->currentObjects(),
                                           name));
    }

    undoStack->endMacro();
}

void PropertiesDock::renameProperty()
{
    QtBrowserItem *item = mPropertyBrowser->currentItem();
    if (!mPropertyBrowser->isCustomPropertyItem(item))
        return;

    const QString oldName = item->property()->propertyName();

    QInputDialog *dialog = new QInputDialog(mPropertyBrowser);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setInputMode(QInputDialog::TextInput);
    dialog->setLabelText(tr("Name:"));
    dialog->setTextValue(oldName);
    dialog->setWindowTitle(tr("Rename Property"));
    connect(dialog, &QInputDialog::textValueSelected, this, &PropertiesDock::renamePropertyTo);
    dialog->open();
}

void PropertiesDock::renamePropertyTo(const QString &name)
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

    const QList<QtBrowserItem *> items = mPropertyBrowser->selectedItems();
    const bool customPropertiesSelected = !items.isEmpty() && mPropertyBrowser->allCustomPropertyItems(items);

    bool currentObjectHasAllProperties = true;
    QStringList propertyNames;
    for (QtBrowserItem *item : items) {
        const QString propertyName = item->property()->propertyName();
        propertyNames.append(propertyName);

        if (!object->hasProperty(propertyName))
            currentObjectHasAllProperties = false;
    }

    QMenu contextMenu(mPropertyBrowser);
    QAction *cutAction = contextMenu.addAction(tr("Cu&t"), this, &PropertiesDock::cutProperties);
    QAction *copyAction = contextMenu.addAction(tr("&Copy"), this, &PropertiesDock::copyProperties);
    QAction *pasteAction = contextMenu.addAction(tr("&Paste"), this, &PropertiesDock::pasteProperties);
    contextMenu.addSeparator();
    QMenu *convertMenu = contextMenu.addMenu(tr("Convert To"));
    QAction *renameAction = contextMenu.addAction(tr("Rename..."), this, &PropertiesDock::renameProperty);
    QAction *removeAction = contextMenu.addAction(tr("Remove"), this, &PropertiesDock::removeProperties);

    cutAction->setShortcuts(QKeySequence::Cut);
    cutAction->setIcon(QIcon(QLatin1String(":/images/16/edit-cut.png")));
    cutAction->setEnabled(customPropertiesSelected && currentObjectHasAllProperties);
    copyAction->setShortcuts(QKeySequence::Copy);
    copyAction->setIcon(QIcon(QLatin1String(":/images/16/edit-copy.png")));
    copyAction->setEnabled(customPropertiesSelected && currentObjectHasAllProperties);
    pasteAction->setShortcuts(QKeySequence::Paste);
    pasteAction->setIcon(QIcon(QLatin1String(":/images/16/edit-paste.png")));
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


    if (customPropertiesSelected) {
        const int convertTo[] = {
            QVariant::Bool,
            QVariant::Color,
            QVariant::Double,
            filePathTypeId(),
            QVariant::Int,
            QVariant::String
        };

        for (int toType : convertTo) {
            bool someDifferentType = false;
            bool allCanConvert = true;

            for (const QString &propertyName : propertyNames) {
                QVariant propertyValue = object->property(propertyName);

                if (propertyValue.userType() != toType)
                    someDifferentType = true;

                if (!propertyValue.convert(toType)) {
                    allCanConvert = false;
                    break;
                }
            }

            if (someDifferentType && allCanConvert) {
                QAction *action = convertMenu->addAction(typeToName(toType));
                action->setData(toType);
            }
        }
    }

    convertMenu->setEnabled(!convertMenu->actions().isEmpty());

    const QPoint globalPos = mPropertyBrowser->mapToGlobal(pos);
    const QAction *selectedItem = contextMenu.exec(globalPos);

    if (selectedItem && selectedItem->parentWidget() == convertMenu) {
        QUndoStack *undoStack = mDocument->undoStack();
        undoStack->beginMacro(tr("Convert Property/Properties", nullptr, items.size()));

        for (const QString &propertyName : propertyNames) {
            QVariant propertyValue = object->property(propertyName);

            int toType = selectedItem->data().toInt();
            propertyValue.convert(toType);

            undoStack->push(new SetProperty(mDocument,
                                            mDocument->currentObjects(),
                                            propertyName, propertyValue));
        }

        undoStack->endMacro();
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
        removeProperties();
    } else if (event->matches(QKeySequence::Cut)) {
        cutProperties();
    } else if (event->matches(QKeySequence::Copy)) {
        copyProperties();
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

} // namespace Tiled
