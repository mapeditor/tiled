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
#include "compression.h"
#include "mapdocument.h"
#include "preferences.h"
#include "propertybrowser.h"
#include "tilesetchanges.h"
#include "tilesetdocument.h"
#include "utils.h"
#include "varianteditor.h"

#include <QAction>
#include <QComboBox>
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

    registerEditorFactories();
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

static QStringList classNamesFor(const Object &object)
{
    QStringList names;
    for (const auto type : Object::propertyTypes())
        if (type->isClass())
            if (static_cast<const ClassPropertyType*>(type)->isClassFor(object))
                names.append(type->name);
    return names;
}

class ClassProperty : public Property
{
    Q_OBJECT

public:
    ClassProperty(Document *document, Object *object, QObject *parent = nullptr)
        : Property(tr("Class"), parent)
        , mDocument(document)
        , mObject(object)
    {
        connect(mDocument, &Document::changed,
                this, &ClassProperty::onChanged);
    }

    QVariant value() const override { return mObject->className(); }
    void setValue(const QVariant &value) override
    {
        QUndoStack *undoStack = mDocument->undoStack();
        undoStack->push(new ChangeClassName(mDocument,
                                            { mObject },    // todo: add support for changing multiple objects
                                            value.toString()));
    }

    QWidget *createEditor(QWidget *parent) override
    {
        auto editor = new QComboBox(parent);
        editor->setEditable(true);
        editor->addItems(classNamesFor(*mObject));
        auto syncEditor = [this, editor] {
            const QSignalBlocker blocker(editor);
            editor->setCurrentText(value().toString());
        };
        syncEditor();
        connect(this, &Property::valueChanged, editor, syncEditor);
        connect(editor, &QComboBox::currentTextChanged, this, &Property::setValue);
        connect(Preferences::instance(), &Preferences::propertyTypesChanged,
                editor, [this,editor] {
            editor->clear();
            editor->addItems(classNamesFor(*mObject));
        });
        return editor;
    }

private:
    void onChanged(const ChangeEvent &event)
    {
        if (event.type != ChangeEvent::ObjectsChanged)
            return;

        const auto objectsEvent = static_cast<const ObjectsChangeEvent&>(event);
        if (!objectsEvent.objects.contains(mObject))
            return;

        if (objectsEvent.properties & ObjectsChangeEvent::ClassProperty)
            emit valueChanged();
    }

    Document *mDocument;
    Object *mObject;
};

class MapSizeProperty : public AbstractProperty
{
    Q_OBJECT

public:
    MapSizeProperty(MapDocument *mapDocument, EditorFactory *editorFactory,
                    QObject *parent = nullptr)
        : AbstractProperty(tr("Map Size"), editorFactory, parent)
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
    TileSizeProperty(MapDocument *mapDocument,
                     EditorFactory *editorFactory,
                     QObject *parent = nullptr)
        : AbstractProperty(tr("Tile Size"), editorFactory, parent)
        , mMapDocument(mapDocument)
    {
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
    MapDocument *mMapDocument;
};

class MapProperties : public QObject
{
    Q_OBJECT

public:
    MapProperties(MapDocument *mapDocument,
                  ValueTypeEditorFactory *editorFactory,
                  QObject *parent = nullptr)
        : QObject(parent)
        , mMapDocument(mapDocument)
        , mSizeProperty(new MapSizeProperty(mapDocument, editorFactory, this))
        , mTileSizeProperty(new TileSizeProperty(mapDocument, editorFactory, this))
    {
        mClassProperty = new ClassProperty(mMapDocument, mMapDocument->map());

        mOrientationProperty = editorFactory->createProperty(
                    tr("Orientation"),
                    [this]() {
                        return QVariant::fromValue(map()->orientation());
                    },
                    [this](const QVariant &value) {
                        auto orientation = static_cast<Map::Orientation>(value.toInt());
                        push(new ChangeMapProperty(mMapDocument, orientation));
                    });

        mInfiniteProperty = editorFactory->createProperty(
                    tr("Infinite"),
                    [this]() {
                        return map()->infinite();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeMapProperty(mMapDocument,
                                                   Map::InfiniteProperty,
                                                   value.toInt()));
                    });

        mHexSideLengthProperty = editorFactory->createProperty(
                    tr("Hex Side Length"),
                    [this]() {
                        return map()->hexSideLength();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeMapProperty(mMapDocument,
                                                   Map::HexSideLengthProperty,
                                                   value.toInt()));
                    });

        mStaggerAxisProperty = editorFactory->createProperty(
                    tr("Stagger Axis"),
                    [this]() {
                        return QVariant::fromValue(map()->staggerAxis());
                    },
                    [this](const QVariant &value) {
                        auto staggerAxis = static_cast<Map::StaggerAxis>(value.toInt());
                        push(new ChangeMapProperty(mMapDocument, staggerAxis));
                    });

        mStaggerIndexProperty = editorFactory->createProperty(
                    tr("Stagger Index"),
                    [this]() {
                        return QVariant::fromValue(map()->staggerIndex());
                    },
                    [this](const QVariant &value) {
                        auto staggerIndex = static_cast<Map::StaggerIndex>(value.toInt());
                        push(new ChangeMapProperty(mMapDocument, staggerIndex));
                    });

        mParallaxOriginProperty = editorFactory->createProperty(
                    tr("Parallax Origin"),
                    [this]() {
                        return map()->parallaxOrigin();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeMapProperty(mMapDocument, value.value<QPointF>()));
                    });

        mLayerDataFormatProperty = editorFactory->createProperty(
                    tr("Layer Data Format"),
                    [this]() {
                        return QVariant::fromValue(map()->layerDataFormat());
                    },
                    [this](const QVariant &value) {
                        auto layerDataFormat = static_cast<Map::LayerDataFormat>(value.toInt());
                        push(new ChangeMapProperty(mMapDocument, layerDataFormat));
                    });

        mChunkSizeProperty = editorFactory->createProperty(
                    tr("Output Chunk Size"),
                    [this]() {
                        return map()->chunkSize();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeMapProperty(mMapDocument, value.toSize()));
                    });

        mRenderOrderProperty = editorFactory->createProperty(
                    tr("Tile Render Order"),
                    [this]() {
                        return QVariant::fromValue(map()->renderOrder());
                    },
                    [this](const QVariant &value) {
                        auto renderOrder = static_cast<Map::RenderOrder>(value.toInt());
                        push(new ChangeMapProperty(mMapDocument, renderOrder));
                    });

        mCompressionLevelProperty = editorFactory->createProperty(
                    tr("Compression Level"),
                    [this]() {
                        return map()->compressionLevel();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeMapProperty(mMapDocument, value.toInt()));
                    });

        mBackgroundColorProperty = editorFactory->createProperty(
                    tr("Background Color"),
                    [this]() {
                        return map()->backgroundColor();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeMapProperty(mMapDocument, value.value<QColor>()));
                    });

        updateEnabledState();
        connect(mMapDocument, &Document::changed,
                this, &MapProperties::onChanged);
    }

    void populateEditor(VariantEditor *editor)
    {
        editor->addHeader(tr("Map"));
        editor->addProperty(mClassProperty);
        editor->addSeparator();
        editor->addProperty(mOrientationProperty);
        editor->addProperty(mSizeProperty);
        editor->addProperty(mInfiniteProperty);
        editor->addProperty(mTileSizeProperty);
        editor->addProperty(mHexSideLengthProperty);
        editor->addProperty(mStaggerAxisProperty);
        editor->addProperty(mStaggerIndexProperty);
        editor->addSeparator();
        editor->addProperty(mParallaxOriginProperty);
        editor->addSeparator();
        editor->addProperty(mLayerDataFormatProperty);
        editor->addProperty(mChunkSizeProperty);
        editor->addProperty(mCompressionLevelProperty);
        editor->addSeparator();
        editor->addProperty(mRenderOrderProperty);
        editor->addProperty(mBackgroundColorProperty);
    }

private:
    void onChanged(const ChangeEvent &event)
    {
        if (event.type != ChangeEvent::MapChanged)
            return;

        const auto property = static_cast<const MapChangeEvent&>(event).property;
        switch (property) {
        case Map::TileWidthProperty:
        case Map::TileHeightProperty:
            emit mTileSizeProperty->valueChanged();
            break;
        case Map::InfiniteProperty:
            emit mInfiniteProperty->valueChanged();
            break;
        case Map::HexSideLengthProperty:
            emit mHexSideLengthProperty->valueChanged();
            break;
        case Map::StaggerAxisProperty:
            emit mStaggerAxisProperty->valueChanged();
            break;
        case Map::StaggerIndexProperty:
            emit mStaggerIndexProperty->valueChanged();
            break;
        case Map::ParallaxOriginProperty:
            emit mParallaxOriginProperty->valueChanged();
            break;
        case Map::OrientationProperty:
            emit mOrientationProperty->valueChanged();
            break;
        case Map::RenderOrderProperty:
            emit mRenderOrderProperty->valueChanged();
            break;
        case Map::BackgroundColorProperty:
            emit mBackgroundColorProperty->valueChanged();
            break;
        case Map::LayerDataFormatProperty:
            emit mLayerDataFormatProperty->valueChanged();
            break;
        case Map::CompressionLevelProperty:
            emit mCompressionLevelProperty->valueChanged();
            break;
        case Map::ChunkSizeProperty:
            emit mChunkSizeProperty->valueChanged();
            break;
        }

        updateEnabledState();
    }

    void updateEnabledState()
    {
        const auto orientation = map()->orientation();
        const bool stagger = orientation == Map::Staggered || orientation == Map::Hexagonal;

        mHexSideLengthProperty->setEnabled(orientation == Map::Hexagonal);
        mStaggerAxisProperty->setEnabled(stagger);
        mStaggerIndexProperty->setEnabled(stagger);
        mRenderOrderProperty->setEnabled(orientation == Map::Orthogonal);
        mChunkSizeProperty->setEnabled(map()->infinite());

        switch (map()->layerDataFormat()) {
        case Map::XML:
        case Map::Base64:
        case Map::CSV:
            mCompressionLevelProperty->setEnabled(false);
            break;
        case Map::Base64Gzip:
        case Map::Base64Zlib:
        case Map::Base64Zstandard:
            mCompressionLevelProperty->setEnabled(true);
            break;
        }
    }

    void push(QUndoCommand *command)
    {
        mMapDocument->undoStack()->push(command);
    }

    Map *map() const
    {
        return mMapDocument->map();
    }

    MapDocument *mMapDocument;
    Property *mClassProperty;
    Property *mOrientationProperty;
    Property *mSizeProperty;
    Property *mTileSizeProperty;
    Property *mInfiniteProperty;
    Property *mHexSideLengthProperty;
    Property *mStaggerAxisProperty;
    Property *mStaggerIndexProperty;
    Property *mParallaxOriginProperty;
    Property *mLayerDataFormatProperty;
    Property *mChunkSizeProperty;
    Property *mRenderOrderProperty;
    Property *mCompressionLevelProperty;
    Property *mBackgroundColorProperty;
};

class TilesetProperties : public QObject
{
    Q_OBJECT

public:
    TilesetProperties(TilesetDocument *tilesetDocument,
                      ValueTypeEditorFactory *editorFactory,
                      QObject *parent = nullptr)
        : QObject(parent)
        , mTilesetDocument(tilesetDocument)
    {
        mNameProperty = editorFactory->createProperty(
                    tr("Name"),
                    [this]() {
                        return mTilesetDocument->tileset()->name();
                    },
                    [this](const QVariant &value) {
                        push(new RenameTileset(mTilesetDocument, value.toString()));
                    });

        mClassProperty = new ClassProperty(tilesetDocument, tilesetDocument->tileset().data());

        mObjectAlignmentProperty = editorFactory->createProperty(
                    tr("Object Alignment"),
                    [this]() {
                        return QVariant::fromValue(tileset()->objectAlignment());
                    },
                    [this](const QVariant &value) {
                        const auto objectAlignment = static_cast<Alignment>(value.toInt());
                        push(new ChangeTilesetObjectAlignment(mTilesetDocument, objectAlignment));
                    });

        mTileOffsetProperty = editorFactory->createProperty(
                    tr("Drawing Offset"),
                    [this]() {
                        return tileset()->tileOffset();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeTilesetTileOffset(mTilesetDocument, value.value<QPoint>()));
                    });

        mTileRenderSizeProperty = editorFactory->createProperty(
                    tr("Tile Render Size"),
                    [this]() {
                        return QVariant::fromValue(tileset()->tileRenderSize());
                    },
                    [this](const QVariant &value) {
                        const auto tileRenderSize = static_cast<Tileset::TileRenderSize>(value.toInt());
                        push(new ChangeTilesetTileRenderSize(mTilesetDocument, tileRenderSize));
                    });

        mFillModeProperty = editorFactory->createProperty(
                    tr("Fill Mode"),
                    [this]() {
                        return QVariant::fromValue(tileset()->fillMode());
                    },
                    [this](const QVariant &value) {
                        const auto fillMode = static_cast<Tileset::FillMode>(value.toInt());
                        push(new ChangeTilesetFillMode(mTilesetDocument, fillMode));
                    });

        mBackgroundColorProperty = editorFactory->createProperty(
                    tr("Background Color"),
                    [this]() {
                        return tileset()->backgroundColor();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeTilesetBackgroundColor(mTilesetDocument, value.value<QColor>()));
                    });

        mOrientationProperty = editorFactory->createProperty(
                    tr("Orientation"),
                    [this]() {
                        return QVariant::fromValue(tileset()->orientation());
                    },
                    [this](const QVariant &value) {
                        const auto orientation = static_cast<Tileset::Orientation>(value.toInt());
                        push(new ChangeTilesetOrientation(mTilesetDocument, orientation));
                    });

        mGridSizeProperty = editorFactory->createProperty(
                    tr("Grid Size"),
                    [this]() {
                        return tileset()->gridSize();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeTilesetGridSize(mTilesetDocument, value.toSize()));
                    });

        // todo: needs 1 as minimum value
        mColumnCountProperty = editorFactory->createProperty(
                    tr("Columns"),
                    [this]() {
                        return tileset()->columnCount();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeTilesetColumnCount(mTilesetDocument, value.toInt()));
                    });

        // todo: this needs a custom widget
        mAllowedTransformationsProperty = editorFactory->createProperty(
                    tr("Allowed Transformations"),
                    [this]() {
                        return QVariant::fromValue(tileset()->transformationFlags());
                    },
                    [this](const QVariant &value) {
                        const auto flags = static_cast<Tileset::TransformationFlags>(value.toInt());
                        push(new ChangeTilesetTransformationFlags(mTilesetDocument, flags));
                    });

        // todo: this needs a custom widget
        mImageProperty = editorFactory->createProperty(
                    tr("Image"),
                    [this]() {
                        return tileset()->imageSource().toString();
                    },
                    [](const QVariant &) {
                        // push(new ChangeTilesetImage(mTilesetDocument, value.toString()));
                    });

        updateEnabledState();
        connect(mTilesetDocument, &Document::changed,
                this, &TilesetProperties::onChanged);

        connect(mTilesetDocument, &TilesetDocument::tilesetNameChanged,
                mNameProperty, &Property::valueChanged);
        connect(mTilesetDocument, &TilesetDocument::tilesetTileOffsetChanged,
                mTileOffsetProperty, &Property::valueChanged);
        connect(mTilesetDocument, &TilesetDocument::tilesetObjectAlignmentChanged,
                mObjectAlignmentProperty, &Property::valueChanged);
        connect(mTilesetDocument, &TilesetDocument::tilesetChanged,
                this, &TilesetProperties::onTilesetChanged);
    }

    void populateEditor(VariantEditor *editor)
    {
        editor->addHeader(tr("Tileset"));
        editor->addProperty(mNameProperty);
        editor->addProperty(mClassProperty);
        editor->addSeparator();
        editor->addProperty(mObjectAlignmentProperty);
        editor->addProperty(mTileOffsetProperty);
        editor->addProperty(mTileRenderSizeProperty);
        editor->addProperty(mFillModeProperty);
        editor->addProperty(mBackgroundColorProperty);
        editor->addProperty(mOrientationProperty);
        editor->addProperty(mGridSizeProperty);
        editor->addProperty(mColumnCountProperty);
        editor->addProperty(mAllowedTransformationsProperty);
        editor->addProperty(mImageProperty);
    }

private:
    void onChanged(const ChangeEvent &event)
    {
        if (event.type != ChangeEvent::TilesetChanged)
            return;

        const auto property = static_cast<const TilesetChangeEvent&>(event).property;
        switch (property) {
        case Tileset::FillModeProperty:
            emit mFillModeProperty->valueChanged();
            break;
        case Tileset::TileRenderSizeProperty:
            emit mTileRenderSizeProperty->valueChanged();
            break;
        }
    }

    void onTilesetChanged(Tileset *)
    {
        // the following properties have no specific change events
        emit mBackgroundColorProperty->valueChanged();
        emit mOrientationProperty->valueChanged();
        emit mGridSizeProperty->valueChanged();
        emit mColumnCountProperty->valueChanged();
        emit mAllowedTransformationsProperty->valueChanged();
        emit mImageProperty->valueChanged();
    }

    void updateEnabledState()
    {
        const bool collection = tileset()->isCollection();
        mImageProperty->setEnabled(!collection);
        mColumnCountProperty->setEnabled(collection);
    }

    void push(QUndoCommand *command)
    {
        mTilesetDocument->undoStack()->push(command);
    }

    Tileset *tileset() const
    {
        return mTilesetDocument->tileset().data();
    }

    TilesetDocument *mTilesetDocument;
    Property *mNameProperty;
    Property *mClassProperty;
    Property *mObjectAlignmentProperty;
    Property *mTileOffsetProperty;
    Property *mTileRenderSizeProperty;
    Property *mFillModeProperty;
    Property *mBackgroundColorProperty;
    Property *mOrientationProperty;
    Property *mGridSizeProperty;
    Property *mColumnCountProperty;
    Property *mAllowedTransformationsProperty;
    Property *mImageProperty;
};


void PropertiesWidget::currentObjectChanged(Object *object)
{
    mPropertyBrowser->clear();
    delete mPropertiesObject;
    mPropertiesObject = nullptr;

    if (object) {
        switch (object->typeId()) {
        case Object::LayerType:
        case Object::MapObjectType:
            break;
        case Object::MapType: {
            auto mapDocument = static_cast<MapDocument*>(mDocument);
            auto properties = new MapProperties(mapDocument, mDefaultEditorFactory.get(), this);
            properties->populateEditor(mPropertyBrowser);
            mPropertiesObject = properties;
            break;
        }
        case Object::TilesetType: {
            auto tilesetDocument = static_cast<TilesetDocument*>(mDocument);
            auto properties = new TilesetProperties(tilesetDocument,
                                                    mDefaultEditorFactory.get(), this);
            properties->populateEditor(mPropertyBrowser);
            mPropertiesObject = properties;
        }
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

void PropertiesWidget::registerEditorFactories()
{
    registerEditorFactory(qMetaTypeId<Alignment>(),
                          std::make_unique<EnumEditorFactory>(
                              QStringList {
                                  tr("Unspecified"),
                                  tr("Top Left"),
                                  tr("Top"),
                                  tr("Top Right"),
                                  tr("Left"),
                                  tr("Center"),
                                  tr("Right"),
                                  tr("Bottom Left"),
                                  tr("Bottom"),
                                  tr("Bottom Right"),
                              }));

    // We leave out the "Unknown" orientation, because it shouldn't occur here
    registerEditorFactory(qMetaTypeId<Map::Orientation>(),
                          std::make_unique<EnumEditorFactory>(
                              QStringList {
                                  tr("Orthogonal"),
                                  tr("Isometric"),
                                  tr("Isometric (Staggered)"),
                                  tr("Hexagonal (Staggered)"),
                              },
                              QList<int> {
                                  Map::Orthogonal,
                                  Map::Isometric,
                                  Map::Staggered,
                                  Map::Hexagonal,
                              }));

    registerEditorFactory(qMetaTypeId<Map::StaggerAxis>(),
                          std::make_unique<EnumEditorFactory>(
                              QStringList {
                                  tr("X"),
                                  tr("Y"),
                              }));

    registerEditorFactory(qMetaTypeId<Map::StaggerIndex>(),
                          std::make_unique<EnumEditorFactory>(
                              QStringList {
                                  tr("Odd"),
                                  tr("Even"),
                              }));

    QStringList layerFormatNames = {
        QCoreApplication::translate("PreferencesDialog", "XML (deprecated)"),
        QCoreApplication::translate("PreferencesDialog", "Base64 (uncompressed)"),
        QCoreApplication::translate("PreferencesDialog", "Base64 (gzip compressed)"),
        QCoreApplication::translate("PreferencesDialog", "Base64 (zlib compressed)"),
    };
    QList<int> layerFormatValues = {
        Map::XML,
        Map::Base64,
        Map::Base64Gzip,
        Map::Base64Zlib,
    };

    if (compressionSupported(Zstandard)) {
        layerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "Base64 (Zstandard compressed)"));
        layerFormatValues.append(Map::Base64Zstandard);
    }

    layerFormatNames.append(QCoreApplication::translate("PreferencesDialog", "CSV"));
    layerFormatValues.append(Map::CSV);

    registerEditorFactory(qMetaTypeId<Map::LayerDataFormat>(),
                          std::make_unique<EnumEditorFactory>(layerFormatNames, layerFormatValues));

    registerEditorFactory(qMetaTypeId<Map::RenderOrder>(),
                          std::make_unique<EnumEditorFactory>(
                              QStringList {
                                  tr("Right Down"),
                                  tr("Right Up"),
                                  tr("Left Down"),
                                  tr("Left Up"),
                              }));

    registerEditorFactory(qMetaTypeId<Tileset::Orientation>(),
                          std::make_unique<EnumEditorFactory>(
                              QStringList {
                                  tr("Orthogonal"),
                                  tr("Isometric"),
                              }));

    registerEditorFactory(qMetaTypeId<Tileset::TileRenderSize>(),
                          std::make_unique<EnumEditorFactory>(
                              QStringList {
                                  tr("Tile Size"),
                                  tr("Map Grid Size"),
                              }));

    registerEditorFactory(qMetaTypeId<Tileset::FillMode>(),
                          std::make_unique<EnumEditorFactory>(
                              QStringList {
                                  tr("Stretch"),
                                  tr("Preserve Aspect Ratio"),
                              }));
}

void PropertiesWidget::registerEditorFactory(int type, std::unique_ptr<EditorFactory> factory)
{
    mDefaultEditorFactory->registerEditorFactory(type, std::move(factory));
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
