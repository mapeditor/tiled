/*
 * propertieswidget.cpp
 * Copyright 2013-2023, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "propertieswidget.h"

#include "actionmanager.h"
#include "addpropertydialog.h"
#include "changemapproperty.h"
#include "changeproperties.h"
#include "clipboardmanager.h"
#include "mapdocument.h"
#include "propertybrowser.h"
#include "utils.h"
#include "varianteditor.h"

#include <QAction>
#include <QCoreApplication>
#include <QEvent>
#include <QFileInfo>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QPushButton>
#include <QToolBar>
#include <QUndoStack>
#include <QVBoxLayout>

namespace Tiled {

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QWidget{parent}
    , mDocument(nullptr)
    , mPropertyBrowser(new VariantEditor(this))
    , mDefaultEditorFactory(std::make_unique<ValueTypeEditorFactory>())
{
    mActionAddProperty = new QAction(this);
    mActionAddProperty->setEnabled(false);
    mActionAddProperty->setIcon(QIcon(QLatin1String(":/images/16/add.png")));
    connect(mActionAddProperty, &QAction::triggered,
            this, &PropertiesWidget::openAddPropertyDialog);

    mActionRemoveProperty = new QAction(this);
    mActionRemoveProperty->setEnabled(false);
    mActionRemoveProperty->setIcon(QIcon(QLatin1String(":/images/16/remove.png")));
    mActionRemoveProperty->setShortcuts(QKeySequence::Delete);
    connect(mActionRemoveProperty, &QAction::triggered,
            this, &PropertiesWidget::removeProperties);

    mActionRenameProperty = new QAction(this);
    mActionRenameProperty->setEnabled(false);
    mActionRenameProperty->setIcon(QIcon(QLatin1String(":/images/16/rename.png")));
    connect(mActionRenameProperty, &QAction::triggered,
            this, &PropertiesWidget::renameProperty);

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

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(mPropertyBrowser);
    layout->addWidget(toolBar);
    setLayout(layout);

    mPropertyBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mPropertyBrowser, &PropertyBrowser::customContextMenuRequested,
            this, &PropertiesWidget::showContextMenu);
    // connect(mPropertyBrowser, &PropertyBrowser::selectedItemsChanged,
    //         this, &PropertiesWidget::updateActions);

    retranslateUi();
}

PropertiesWidget::~PropertiesWidget()
{
    // Disconnect to avoid crashing due to signals emitted during destruction
    mPropertyBrowser->disconnect(this);
}

void PropertiesWidget::setDocument(Document *document)
{
    if (mDocument == document)
        return;

    if (mDocument)
        mDocument->disconnect(this);

    mDocument = document;
    // mPropertyBrowser->setDocument(document);

    if (document) {
        connect(document, &Document::currentObjectChanged,
                this, &PropertiesWidget::currentObjectChanged);
        connect(document, &Document::editCurrentObject,
                this, &PropertiesWidget::bringToFront);

        connect(document, &Document::propertyAdded,
                this, &PropertiesWidget::updateActions);
        connect(document, &Document::propertyRemoved,
                this, &PropertiesWidget::updateActions);

        currentObjectChanged(document->currentObject());
    } else {
        currentObjectChanged(nullptr);
    }
}

void PropertiesWidget::selectCustomProperty(const QString &name)
{
    // mPropertyBrowser->selectCustomProperty(name);
}

static bool anyObjectHasProperty(const QList<Object*> &objects, const QString &name)
{
    for (Object *obj : objects) {
        if (obj->hasProperty(name))
            return true;
    }
    return false;
}

class MapOrientationProperty : public EnumProperty
{
    Q_OBJECT

public:
    MapOrientationProperty(MapDocument *mapDocument)
        : EnumProperty(tr("Orientation"))
        , mMapDocument(mapDocument)
    {
        setEnumNames({
                         tr("Orthogonal"),
                         tr("Isometric"),
                         tr("Isometric (Staggered)"),
                         tr("Hexagonal (Staggered)"),
                     });
        setEnumValues({
                          Map::Orthogonal,
                          Map::Isometric,
                          Map::Staggered,
                          Map::Hexagonal,
                      });

        connect(mMapDocument, &MapDocument::mapChanged,
                this, &Property::valueChanged);
    }

    QVariant value() const override
    {
        return mMapDocument->map()->orientation();
    }

    void setValue(const QVariant &value) override
    {
        Map::Orientation orientation = static_cast<Map::Orientation>(value.toInt());
        auto command = new ChangeMapProperty(mMapDocument, orientation);
        mMapDocument->undoStack()->push(command);
    }

private:
    MapDocument *mMapDocument;
};

class MapSizeProperty : public AbstractProperty
{
    Q_OBJECT

public:
    MapSizeProperty(MapDocument *mapDocument, EditorFactory *editorFactory)
        : AbstractProperty(tr("Map Size"), editorFactory)
        , mMapDocument(mapDocument)
    {
        connect(mMapDocument, &MapDocument::mapChanged,
                this, &Property::valueChanged);
    }

    QVariant value() const override { return mMapDocument->map()->size(); }
    void setValue(const QVariant &) override {};

    QWidget *createEditor(QWidget *parent) override
    {
        auto widget = new QWidget(parent);
        auto layout = new QVBoxLayout(widget);
        auto valueEdit = AbstractProperty::createEditor(widget);
        auto resizeButton = new QPushButton(tr("Resize Map"), widget);

        valueEdit->setEnabled(false);
        layout->setContentsMargins(QMargins());
        layout->addWidget(valueEdit);
        layout->addWidget(resizeButton, 0, Qt::AlignLeft);

        connect(resizeButton, &QPushButton::clicked, [] {
            ActionManager::action("ResizeMap")->trigger();
        });

        return widget;
    }

private:
    MapDocument *mMapDocument;
};

class TileSizeProperty : public AbstractProperty
{
    Q_OBJECT

public:
    TileSizeProperty(MapDocument *mapDocument, EditorFactory *editorFactory)
        : AbstractProperty(tr("Tile Size"), editorFactory)
        , mMapDocument(mapDocument)
    {
        connect(mMapDocument, &Document::changed,
                this, &TileSizeProperty::onChanged);
    }

    QVariant value() const override
    {
        return mMapDocument->map()->tileSize();
    }

    void setValue(const QVariant &value) override
    {
        auto oldSize = mMapDocument->map()->tileSize();
        auto newSize = value.toSize();

        if (oldSize.width() != newSize.width()) {
            auto command = new ChangeMapProperty(mMapDocument,
                                                 Map::TileWidthProperty,
                                                 newSize.width());
            mMapDocument->undoStack()->push(command);
        }

        if (oldSize.height() != newSize.height()) {
            auto command = new ChangeMapProperty(mMapDocument,
                                                 Map::TileHeightProperty,
                                                 newSize.height());
            mMapDocument->undoStack()->push(command);
        }
    };

private:
    void onChanged(const ChangeEvent &event)
    {
        if (event.type != ChangeEvent::MapChanged)
            return;

        const auto property = static_cast<const MapChangeEvent&>(event).property;
        if (property == Map::TileWidthProperty || property == Map::TileHeightProperty)
            emit valueChanged();
    }

    MapDocument *mMapDocument;
};


void PropertiesWidget::currentObjectChanged(Object *object)
{
    // mPropertyBrowser->setObject(object);
    mPropertyBrowser->clear();

    if (object) {
        switch (object->typeId()) {
        case Object::LayerType:
        case Object::MapObjectType:
            break;
        case Object::MapType: {
            Map *map = static_cast<Map*>(object);
            auto mapDocument = static_cast<MapDocument*>(mDocument);

            mPropertyBrowser->addHeader(tr("Map"));
            mPropertyBrowser->addProperty(new MapOrientationProperty(mapDocument));
            mPropertyBrowser->addProperty(new MapSizeProperty(mapDocument, mDefaultEditorFactory.get()));
            mPropertyBrowser->addProperty(new TileSizeProperty(mapDocument, mDefaultEditorFactory.get()));
            // todo: infinite
            // todo: hex side length
            // todo: stagger axis
            // todo: stagger index
            // todo: parallax origin
            // todo: layer data format
            // todo: chunk size
            // todo: tile render order
            // todo: compression level
            // todo: background color

            break;
        }
        case Object::TilesetType:
        case Object::TileType:
        case Object::WangSetType:
        case Object::WangColorType:
        case Object::ProjectType:
        case Object::WorldType:
            break;
        }
    }

    bool editingTileset = mDocument && mDocument->type() == Document::TilesetDocumentType;
    bool isTileset = object && object->isPartOfTileset();
    bool enabled = object && (!isTileset || editingTileset);

    mPropertyBrowser->setEnabled(object);
    mActionAddProperty->setEnabled(enabled);
}

void PropertiesWidget::updateActions()
{
#if 0
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
#endif
}

void PropertiesWidget::cutProperties()
{
    if (copyProperties())
        removeProperties();
}

bool PropertiesWidget::copyProperties()
{
#if 0
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
#endif
    return true;
}

void PropertiesWidget::pasteProperties()
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
        mergeProperties(properties, pastedProperties);

        if (object->properties() != properties) {
            commands.append(new ChangeProperties(mDocument, QString(), object,
                                                 properties));
        }
    }

    if (!commands.isEmpty()) {
        QUndoStack *undoStack = mDocument->undoStack();
        undoStack->beginMacro(QCoreApplication::translate("Tiled::PropertiesDock",
                                                          "Paste Property/Properties",
                                                          nullptr,
                                                          pastedProperties.size()));

        for (QUndoCommand *command : commands)
            undoStack->push(command);

        undoStack->endMacro();
    }
}

void PropertiesWidget::openAddPropertyDialog()
{
    AddPropertyDialog dialog(mPropertyBrowser);
    if (dialog.exec() == AddPropertyDialog::Accepted)
        addProperty(dialog.propertyName(), dialog.propertyValue());
}

void PropertiesWidget::addProperty(const QString &name, const QVariant &value)
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

    // mPropertyBrowser->editCustomProperty(name);
}

void PropertiesWidget::removeProperties()
{
#if 0
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
    undoStack->beginMacro(QCoreApplication::translate("Tiled::PropertiesDock",
                                                      "Remove Property/Properties",
                                                      nullptr,
                                                      propertyNames.size()));

    for (const QString &name : propertyNames) {
        undoStack->push(new RemoveProperty(mDocument,
                                           mDocument->currentObjects(),
                                           name));
    }

    undoStack->endMacro();
#endif
}

void PropertiesWidget::renameProperty()
{
#if 0
    QtBrowserItem *item = mPropertyBrowser->currentItem();
    if (!mPropertyBrowser->isCustomPropertyItem(item))
        return;

    const QString oldName = item->property()->propertyName();

    QInputDialog *dialog = new QInputDialog(mPropertyBrowser);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setInputMode(QInputDialog::TextInput);
    dialog->setLabelText(QCoreApplication::translate("Tiled::PropertiesDock", "Name:"));
    dialog->setTextValue(oldName);
    dialog->setWindowTitle(QCoreApplication::translate("Tiled::PropertiesDock", "Rename Property"));
    connect(dialog, &QInputDialog::textValueSelected, this, &PropertiesWidget::renamePropertyTo);
    dialog->open();
#endif
}

void PropertiesWidget::renamePropertyTo(const QString &name)
{
#if 0
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
#endif
}

void PropertiesWidget::showContextMenu(const QPoint &pos)
{
#if 0
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

    if (customPropertiesSelected && propertyNames.size() == 1) {
        const auto value = object->resolvedProperty(propertyNames.first());
        if (value.userType() == filePathTypeId()) {
            const FilePath filePath = value.value<FilePath>();
            const QString localFile = filePath.url.toLocalFile();

            if (!localFile.isEmpty()) {
                Utils::addOpenContainingFolderAction(contextMenu, localFile);

                if (QFileInfo { localFile }.isFile())
                    Utils::addOpenWithSystemEditorAction(contextMenu, localFile);
            }
        } else if (value.userType() == objectRefTypeId()) {
            if (auto mapDocument = qobject_cast<MapDocument*>(mDocument)) {
                const DisplayObjectRef objectRef(value.value<ObjectRef>(), mapDocument);

                contextMenu.addAction(QCoreApplication::translate("Tiled::PropertiesDock", "Go to Object"), [=] {
                    if (auto object = objectRef.object()) {
                        objectRef.mapDocument->setSelectedObjects({object});
                        emit objectRef.mapDocument->focusMapObjectRequested(object);
                    }
                })->setEnabled(objectRef.object());
            }
        }
    }

    if (!contextMenu.isEmpty())
        contextMenu.addSeparator();

    QAction *cutAction = contextMenu.addAction(QCoreApplication::translate("Tiled::PropertiesDock", "Cu&t"), this, &PropertiesWidget::cutProperties);
    QAction *copyAction = contextMenu.addAction(QCoreApplication::translate("Tiled::PropertiesDock", "&Copy"), this, &PropertiesWidget::copyProperties);
    QAction *pasteAction = contextMenu.addAction(QCoreApplication::translate("Tiled::PropertiesDock", "&Paste"), this, &PropertiesWidget::pasteProperties);
    contextMenu.addSeparator();
    QMenu *convertMenu = nullptr;

    if (customPropertiesSelected) {
        convertMenu = contextMenu.addMenu(QCoreApplication::translate("Tiled::PropertiesDock", "Convert To"));
        contextMenu.addAction(mActionRemoveProperty);
        contextMenu.addAction(mActionRenameProperty);
    } else {
        contextMenu.addAction(mActionAddProperty);
    }

    cutAction->setShortcuts(QKeySequence::Cut);
    cutAction->setIcon(QIcon(QLatin1String(":/images/16/edit-cut.png")));
    cutAction->setEnabled(customPropertiesSelected && currentObjectHasAllProperties);
    copyAction->setShortcuts(QKeySequence::Copy);
    copyAction->setIcon(QIcon(QLatin1String(":/images/16/edit-copy.png")));
    copyAction->setEnabled(customPropertiesSelected && currentObjectHasAllProperties);
    pasteAction->setShortcuts(QKeySequence::Paste);
    pasteAction->setIcon(QIcon(QLatin1String(":/images/16/edit-paste.png")));
    pasteAction->setEnabled(ClipboardManager::instance()->hasProperties());

    Utils::setThemeIcon(cutAction, "edit-cut");
    Utils::setThemeIcon(copyAction, "edit-copy");
    Utils::setThemeIcon(pasteAction, "edit-paste");

    if (convertMenu) {
        const int convertTo[] = {
            QMetaType::Bool,
            QMetaType::QColor,
            QMetaType::Double,
            filePathTypeId(),
            objectRefTypeId(),
            QMetaType::Int,
            QMetaType::QString
        };

        // todo: could include custom property types

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

        convertMenu->setEnabled(!convertMenu->actions().isEmpty());
    }

    ActionManager::applyMenuExtensions(&contextMenu, MenuIds::propertiesViewProperties);

    const QPoint globalPos = mPropertyBrowser->mapToGlobal(pos);
    const QAction *selectedItem = contextMenu.exec(globalPos);

    if (selectedItem && convertMenu && selectedItem->parentWidget() == convertMenu) {
        QUndoStack *undoStack = mDocument->undoStack();
        undoStack->beginMacro(QCoreApplication::translate("Tiled::PropertiesDock", "Convert Property/Properties", nullptr, items.size()));

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
#endif
}

bool PropertiesWidget::event(QEvent *event)
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

    return QWidget::event(event);
}

void PropertiesWidget::keyPressEvent(QKeyEvent *event)
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
        QWidget::keyPressEvent(event);
    }
}

void PropertiesWidget::retranslateUi()
{
    mActionAddProperty->setText(QCoreApplication::translate("Tiled::PropertiesDock", "Add Property"));

    mActionRemoveProperty->setText(QCoreApplication::translate("Tiled::PropertiesDock", "Remove"));
    mActionRemoveProperty->setToolTip(QCoreApplication::translate("Tiled::PropertiesDock", "Remove Property"));

    mActionRenameProperty->setText(QCoreApplication::translate("Tiled::PropertiesDock", "Rename..."));
    mActionRenameProperty->setToolTip(QCoreApplication::translate("Tiled::PropertiesDock", "Rename Property"));
}

} // namespace Tiled

#include "moc_propertieswidget.cpp"
#include "propertieswidget.moc"
