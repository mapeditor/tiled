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
#include "changeimagelayerproperty.h"
#include "changelayer.h"
#include "changemapobject.h"
#include "changemapproperty.h"
#include "changeobjectgroupproperties.h"
#include "changeproperties.h"
#include "changetile.h"
#include "changetileimagesource.h"
#include "changewangcolordata.h"
#include "changewangsetdata.h"
#include "clipboardmanager.h"
#include "compression.h"
#include "mapdocument.h"
#include "objectgroup.h"
#include "objecttemplate.h"
#include "preferences.h"
#include "propertybrowser.h"
#include "tilesetchanges.h"
#include "tilesetdocument.h"
#include "utils.h"
#include "varianteditor.h"
#include "wangoverlay.h"

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

// todo: add support for changing multiple objects
class ClassProperty : public StringProperty
{
    Q_OBJECT

public:
    ClassProperty(Document *document, Object *object, QObject *parent = nullptr)
        : StringProperty(tr("Class"),
                         [this] { return mObject->className(); },
                         [this] (const QString &value) {
                             QUndoStack *undoStack = mDocument->undoStack();
                             undoStack->push(new ChangeClassName(mDocument,
                                                                 { mObject },
                                                                 value));
                         },
                         parent)
        , mDocument(document)
        , mObject(object)
    {
        connect(mDocument, &Document::changed,
                this, &ClassProperty::onChanged);
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

class MapSizeProperty : public SizeProperty
{
    Q_OBJECT

public:
    MapSizeProperty(MapDocument *mapDocument,
                    QObject *parent = nullptr)
        : SizeProperty(tr("Map Size"),
                       [this]{ return mMapDocument->map()->size(); }, {},
                       parent)
        , mMapDocument(mapDocument)
    {
        connect(mMapDocument, &MapDocument::mapChanged,
                this, &Property::valueChanged);
    }

    QWidget *createEditor(QWidget *parent) override
    {
        auto widget = new QWidget(parent);
        auto layout = new QVBoxLayout(widget);
        auto valueEdit = SizeProperty::createEditor(widget);
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

class ObjectProperties : public QObject
{
    Q_OBJECT

public:
    ObjectProperties(Document *document, Object *object, QObject *parent = nullptr)
        : QObject(parent)
        , mDocument(document)
        , mObject(object)
    {
        mClassProperty = new ClassProperty(document, object, this);
    }

    virtual void populateEditor(VariantEditor *)
    {
        // nothing added here due to property grouping
    }

protected:
    void push(QUndoCommand *command)
    {
        mDocument->undoStack()->push(command);
    }

    Document *mDocument;
    Property *mClassProperty;
    Object *mObject;
};


class MapProperties : public ObjectProperties
{
    Q_OBJECT

public:
    MapProperties(MapDocument *document,
                  ValueTypeEditorFactory *editorFactory,
                  QObject *parent = nullptr)
        : ObjectProperties(document, document->map(), parent)
    {
        mOrientationProperty = editorFactory->createProperty(
                    tr("Orientation"),
                    [this]() {
                        return QVariant::fromValue(map()->orientation());
                    },
                    [this](const QVariant &value) {
                        auto orientation = static_cast<Map::Orientation>(value.toInt());
                        push(new ChangeMapProperty(mapDocument(), orientation));
                    });

        mSizeProperty = new MapSizeProperty(mapDocument(), this);

        mTileSizeProperty = new SizeProperty(
                    tr("Tile Size"),
                    [this] {
                        return mapDocument()->map()->tileSize();
                    },
                    [this](const QSize &newSize) {
                        const auto oldSize = mapDocument()->map()->tileSize();

                        if (oldSize.width() != newSize.width()) {
                            push(new ChangeMapProperty(mapDocument(),
                                                       Map::TileWidthProperty,
                                                       newSize.width()));
                        }

                        if (oldSize.height() != newSize.height()) {
                            push(new ChangeMapProperty(mapDocument(),
                                                       Map::TileHeightProperty,
                                                       newSize.height()));
                        }
                    },
                    this);

        mInfiniteProperty = new BoolProperty(
                    tr("Infinite"),
                    [this]() {
                        return map()->infinite();
                    },
                    [this](const bool &value) {
                        push(new ChangeMapProperty(mapDocument(),
                                                   Map::InfiniteProperty,
                                                   value ? 1 : 0));
                    });

        mHexSideLengthProperty = new IntProperty(
                    tr("Hex Side Length"),
                    [this]() {
                        return map()->hexSideLength();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeMapProperty(mapDocument(),
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
                        push(new ChangeMapProperty(mapDocument(), staggerAxis));
                    });

        mStaggerIndexProperty = editorFactory->createProperty(
                    tr("Stagger Index"),
                    [this]() {
                        return QVariant::fromValue(map()->staggerIndex());
                    },
                    [this](const QVariant &value) {
                        auto staggerIndex = static_cast<Map::StaggerIndex>(value.toInt());
                        push(new ChangeMapProperty(mapDocument(), staggerIndex));
                    });

        mParallaxOriginProperty = editorFactory->createProperty(
                    tr("Parallax Origin"),
                    [this]() {
                        return map()->parallaxOrigin();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeMapProperty(mapDocument(), value.value<QPointF>()));
                    });

        mLayerDataFormatProperty = editorFactory->createProperty(
                    tr("Layer Data Format"),
                    [this]() {
                        return QVariant::fromValue(map()->layerDataFormat());
                    },
                    [this](const QVariant &value) {
                        auto layerDataFormat = static_cast<Map::LayerDataFormat>(value.toInt());
                        push(new ChangeMapProperty(mapDocument(), layerDataFormat));
                    });

        mChunkSizeProperty = editorFactory->createProperty(
                    tr("Output Chunk Size"),
                    [this]() {
                        return map()->chunkSize();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeMapProperty(mapDocument(), value.toSize()));
                    });

        mRenderOrderProperty = editorFactory->createProperty(
                    tr("Tile Render Order"),
                    [this]() {
                        return QVariant::fromValue(map()->renderOrder());
                    },
                    [this](const QVariant &value) {
                        auto renderOrder = static_cast<Map::RenderOrder>(value.toInt());
                        push(new ChangeMapProperty(mapDocument(), renderOrder));
                    });

        mCompressionLevelProperty = editorFactory->createProperty(
                    tr("Compression Level"),
                    [this]() {
                        return map()->compressionLevel();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeMapProperty(mapDocument(), value.toInt()));
                    });

        mBackgroundColorProperty = editorFactory->createProperty(
                    tr("Background Color"),
                    [this]() {
                        return map()->backgroundColor();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeMapProperty(mapDocument(), value.value<QColor>()));
                    });

        updateEnabledState();
        connect(document, &Document::changed,
                this, &MapProperties::onChanged);
    }

    void populateEditor(VariantEditor *editor) override
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

    MapDocument *mapDocument() const
    {
        return static_cast<MapDocument*>(mDocument);
    }

    Map *map() const
    {
        return mapDocument()->map();
    }

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

class LayerProperties : public ObjectProperties
{
    Q_OBJECT

public:
    LayerProperties(MapDocument *document, Layer *object, ValueTypeEditorFactory *editorFactory, QObject *parent = nullptr)
        : ObjectProperties(document, object, parent)
    {
        // todo: would be nicer to avoid the SpinBox and use a custom widget
        // might also be nice to embed this in the header instead of using a property
        mIdProperty = editorFactory->createProperty(
                    tr("ID"),
                    [this]() { return layer()->id(); },
                    [](const QVariant &) {});
        mIdProperty->setEnabled(false);

        // todo: the below should be able to apply to all selected layers

        mNameProperty = editorFactory->createProperty(
                    tr("Name"),
                    [this]() { return layer()->name(); },
                    [this](const QVariant &value) {
                        push(new SetLayerName(mapDocument(), { layer() }, value.toString()));
                    });

        mVisibleProperty = editorFactory->createProperty(
                    tr("Visible"),
                    [this]() { return layer()->isVisible(); },
                    [this](const QVariant &value) {
                        push(new SetLayerVisible(mapDocument(), { layer() }, value.toBool()));
                    });

        mLockedProperty = editorFactory->createProperty(
                    tr("Locked"),
                    [this]() { return layer()->isLocked(); },
                    [this](const QVariant &value) {
                        push(new SetLayerLocked(mapDocument(), { layer() }, value.toBool()));
                    });

        // todo: value should be between 0 and 1, and would be nice to use a slider (replacing the one in Layers view)
        // todo: singleStep should be 0.1
        mOpacityProperty = editorFactory->createProperty(
                    tr("Opacity"),
                    [this]() { return layer()->opacity(); },
                    [this](const QVariant &value) {
                        push(new SetLayerOpacity(mapDocument(), { layer() }, value.toReal()));
                    });

        mTintColorProperty = editorFactory->createProperty(
                    tr("Tint Color"),
                    [this]() { return layer()->tintColor(); },
                    [this](const QVariant &value) {
                        push(new SetLayerTintColor(mapDocument(), { layer() }, value.value<QColor>()));
                    });

        mOffsetProperty = editorFactory->createProperty(
                    tr("Offset"),
                    [this]() { return layer()->offset(); },
                    [this](const QVariant &value) {
                        push(new SetLayerOffset(mapDocument(), { layer() }, value.value<QPointF>()));
                    });

        // todo: singleStep should be 0.1
        mParallaxFactorProperty = editorFactory->createProperty(
                    tr("Parallax Factor"),
                    [this]() { return layer()->parallaxFactor(); },
                    [this](const QVariant &value) {
                        push(new SetLayerParallaxFactor(mapDocument(), { layer() }, value.toPointF()));
                    });

        connect(document, &Document::changed,
                this, &LayerProperties::onChanged);
    }

    void populateEditor(VariantEditor *editor) override
    {
        editor->addHeader(tr("Layer"));
        editor->addProperty(mIdProperty);
        editor->addProperty(mNameProperty);
        editor->addProperty(mClassProperty);
        editor->addSeparator();
        editor->addProperty(mVisibleProperty);
        editor->addProperty(mLockedProperty);
        editor->addProperty(mOpacityProperty);
        editor->addProperty(mTintColorProperty);
        editor->addProperty(mOffsetProperty);
        editor->addProperty(mParallaxFactorProperty);
    }

protected:
    virtual void onChanged(const ChangeEvent &event)
    {
        if (event.type != ChangeEvent::LayerChanged)
            return;

        const auto &layerChange = static_cast<const LayerChangeEvent&>(event);
        if (layerChange.layer != layer())
            return;

        if (layerChange.properties & LayerChangeEvent::VisibleProperty)
            emit mVisibleProperty->valueChanged();
        if (layerChange.properties & LayerChangeEvent::LockedProperty)
            emit mLockedProperty->valueChanged();
        if (layerChange.properties & LayerChangeEvent::OpacityProperty)
            emit mOpacityProperty->valueChanged();
        if (layerChange.properties & LayerChangeEvent::TintColorProperty)
            emit mTintColorProperty->valueChanged();
        if (layerChange.properties & LayerChangeEvent::OffsetProperty)
            emit mOffsetProperty->valueChanged();
        if (layerChange.properties & LayerChangeEvent::ParallaxFactorProperty)
            emit mParallaxFactorProperty->valueChanged();
    }

    MapDocument *mapDocument() const
    {
        return static_cast<MapDocument*>(mDocument);
    }

    Layer *layer() const
    {
        return static_cast<Layer*>(mObject);
    }

    Property *mIdProperty;
    Property *mNameProperty;
    Property *mVisibleProperty;
    Property *mLockedProperty;
    Property *mOpacityProperty;
    Property *mTintColorProperty;
    Property *mOffsetProperty;
    Property *mParallaxFactorProperty;
};

class ImageLayerProperties : public LayerProperties
{
    Q_OBJECT

public:
    ImageLayerProperties(MapDocument *document, ImageLayer *object, ValueTypeEditorFactory *editorFactory, QObject *parent = nullptr)
        : LayerProperties(document, object, editorFactory, parent)
    {
        // todo: set a file filter for selecting images (or map files?)
        mImageProperty = editorFactory->createProperty(
                    tr("Image Source"),
                    [this]() { return imageLayer()->imageSource(); },
                    [this](const QVariant &value) {
                        push(new ChangeImageLayerImageSource(mapDocument(), { imageLayer() }, value.toUrl()));
                    });

        mTransparentColorProperty = editorFactory->createProperty(
                    tr("Transparent Color"),
                    [this]() { return imageLayer()->transparentColor(); },
                    [this](const QVariant &value) {
                        push(new ChangeImageLayerTransparentColor(mapDocument(), { imageLayer() }, value.value<QColor>()));
                    });

        // todo: consider merging Repeat X and Y into a single property
        mRepeatXProperty = editorFactory->createProperty(
                    tr("Repeat X"),
                    [this]() { return imageLayer()->repeatX(); },
                    [this](const QVariant &value) {
                        push(new ChangeImageLayerRepeatX(mapDocument(), { imageLayer() }, value.toBool()));
                    });

        mRepeatYProperty = editorFactory->createProperty(
                    tr("Repeat Y"),
                    [this]() { return imageLayer()->repeatY(); },
                    [this](const QVariant &value) {
                        push(new ChangeImageLayerRepeatY(mapDocument(), { imageLayer() }, value.toBool()));
                    });
    }

    void populateEditor(VariantEditor *editor) override
    {
        LayerProperties::populateEditor(editor);
        editor->addHeader(tr("Image Layer"));
        editor->addProperty(mImageProperty);
        editor->addProperty(mTransparentColorProperty);
        editor->addSeparator();
        editor->addProperty(mRepeatXProperty);
        editor->addProperty(mRepeatYProperty);
    }

private:
    void onChanged(const ChangeEvent &event) override
    {
        LayerProperties::onChanged(event);

        if (event.type != ChangeEvent::ImageLayerChanged)
            return;

        const auto &layerChange = static_cast<const ImageLayerChangeEvent&>(event);
        if (layerChange.layer != layer())
            return;

        if (layerChange.properties & ImageLayerChangeEvent::ImageSourceProperty)
            emit mImageProperty->valueChanged();
        if (layerChange.properties & ImageLayerChangeEvent::TransparentColorProperty)
            emit mTransparentColorProperty->valueChanged();
        if (layerChange.properties & ImageLayerChangeEvent::RepeatProperty) {
            emit mRepeatXProperty->valueChanged();
            emit mRepeatYProperty->valueChanged();
        }
    }

    ImageLayer *imageLayer() const
    {
        return static_cast<ImageLayer*>(mObject);
    }

    Property *mImageProperty;
    Property *mTransparentColorProperty;
    Property *mRepeatXProperty;
    Property *mRepeatYProperty;
};

class ObjectGroupProperties : public LayerProperties
{
    Q_OBJECT

public:
    ObjectGroupProperties(MapDocument *document, ObjectGroup *object, ValueTypeEditorFactory *editorFactory, QObject *parent = nullptr)
        : LayerProperties(document, object, editorFactory, parent)
    {
        mColorProperty = editorFactory->createProperty(
                    tr("Color"),
                    [this]() { return objectGroup()->color(); },
                    [this](const QVariant &value) {
                        push(new ChangeObjectGroupColor(mapDocument(), { objectGroup() }, value.value<QColor>()));
                    });

        mDrawOrderProperty = editorFactory->createProperty(
                    tr("Draw Order"),
                    [this]() { return QVariant::fromValue(objectGroup()->drawOrder()); },
                    [this](const QVariant &value) {
                        ObjectGroup::DrawOrder drawOrder = static_cast<ObjectGroup::DrawOrder>(value.toInt());
                        push(new ChangeObjectGroupDrawOrder(mapDocument(), { objectGroup() }, drawOrder));
                    });
    }

    void populateEditor(VariantEditor *editor) override
    {
        LayerProperties::populateEditor(editor);
        editor->addHeader(tr("Object Layer"));
        editor->addProperty(mColorProperty);
        editor->addProperty(mDrawOrderProperty);
    }

private:
    void onChanged(const ChangeEvent &event) override
    {
        LayerProperties::onChanged(event);

        if (event.type != ChangeEvent::ObjectGroupChanged)
            return;

        const auto &layerChange = static_cast<const ObjectGroupChangeEvent&>(event);
        if (layerChange.objectGroup != objectGroup())
            return;

        if (layerChange.properties & ObjectGroupChangeEvent::ColorProperty)
            emit mColorProperty->valueChanged();
        if (layerChange.properties & ObjectGroupChangeEvent::DrawOrderProperty)
            emit mDrawOrderProperty->valueChanged();
    }

    ObjectGroup *objectGroup() const
    {
        return static_cast<ObjectGroup*>(mObject);
    }

    Property *mColorProperty;
    Property *mDrawOrderProperty;
};

class TilesetProperties : public ObjectProperties
{
    Q_OBJECT

public:
    TilesetProperties(TilesetDocument *document,
                      ValueTypeEditorFactory *editorFactory,
                      QObject *parent = nullptr)
        : ObjectProperties(document, document->tileset().data(), parent)
    {
        mNameProperty = editorFactory->createProperty(
                    tr("Name"),
                    [this]() {
                        return tilesetDocument()->tileset()->name();
                    },
                    [this](const QVariant &value) {
                        push(new RenameTileset(tilesetDocument(), value.toString()));
                    });

        mObjectAlignmentProperty = editorFactory->createProperty(
                    tr("Object Alignment"),
                    [this]() {
                        return QVariant::fromValue(tileset()->objectAlignment());
                    },
                    [this](const QVariant &value) {
                        const auto objectAlignment = static_cast<Alignment>(value.toInt());
                        push(new ChangeTilesetObjectAlignment(tilesetDocument(), objectAlignment));
                    });

        mTileOffsetProperty = editorFactory->createProperty(
                    tr("Drawing Offset"),
                    [this]() {
                        return tileset()->tileOffset();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeTilesetTileOffset(tilesetDocument(), value.value<QPoint>()));
                    });

        mTileRenderSizeProperty = editorFactory->createProperty(
                    tr("Tile Render Size"),
                    [this]() {
                        return QVariant::fromValue(tileset()->tileRenderSize());
                    },
                    [this](const QVariant &value) {
                        const auto tileRenderSize = static_cast<Tileset::TileRenderSize>(value.toInt());
                        push(new ChangeTilesetTileRenderSize(tilesetDocument(), tileRenderSize));
                    });

        mFillModeProperty = editorFactory->createProperty(
                    tr("Fill Mode"),
                    [this]() {
                        return QVariant::fromValue(tileset()->fillMode());
                    },
                    [this](const QVariant &value) {
                        const auto fillMode = static_cast<Tileset::FillMode>(value.toInt());
                        push(new ChangeTilesetFillMode(tilesetDocument(), fillMode));
                    });

        mBackgroundColorProperty = editorFactory->createProperty(
                    tr("Background Color"),
                    [this]() {
                        return tileset()->backgroundColor();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeTilesetBackgroundColor(tilesetDocument(), value.value<QColor>()));
                    });

        mOrientationProperty = editorFactory->createProperty(
                    tr("Orientation"),
                    [this]() {
                        return QVariant::fromValue(tileset()->orientation());
                    },
                    [this](const QVariant &value) {
                        const auto orientation = static_cast<Tileset::Orientation>(value.toInt());
                        push(new ChangeTilesetOrientation(tilesetDocument(), orientation));
                    });

        mGridSizeProperty = editorFactory->createProperty(
                    tr("Grid Size"),
                    [this]() {
                        return tileset()->gridSize();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeTilesetGridSize(tilesetDocument(), value.toSize()));
                    });

        // todo: needs 1 as minimum value
        mColumnCountProperty = editorFactory->createProperty(
                    tr("Columns"),
                    [this]() {
                        return tileset()->columnCount();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeTilesetColumnCount(tilesetDocument(), value.toInt()));
                    });

        // todo: this needs a custom widget
        mAllowedTransformationsProperty = editorFactory->createProperty(
                    tr("Allowed Transformations"),
                    [this]() {
                        return QVariant::fromValue(tileset()->transformationFlags());
                    },
                    [this](const QVariant &value) {
                        const auto flags = static_cast<Tileset::TransformationFlags>(value.toInt());
                        push(new ChangeTilesetTransformationFlags(tilesetDocument(), flags));
                    });

        // todo: this needs a custom widget
        mImageProperty = editorFactory->createProperty(
                    tr("Image"),
                    [this]() {
                        return tileset()->imageSource().toString();
                    },
                    [](const QVariant &) {
                        // push(new ChangeTilesetImage(tilesetDocument(), value.toString()));
                    });

        updateEnabledState();
        connect(tilesetDocument(), &Document::changed,
                this, &TilesetProperties::onChanged);

        connect(tilesetDocument(), &TilesetDocument::tilesetNameChanged,
                mNameProperty, &Property::valueChanged);
        connect(tilesetDocument(), &TilesetDocument::tilesetTileOffsetChanged,
                mTileOffsetProperty, &Property::valueChanged);
        connect(tilesetDocument(), &TilesetDocument::tilesetObjectAlignmentChanged,
                mObjectAlignmentProperty, &Property::valueChanged);
        connect(tilesetDocument(), &TilesetDocument::tilesetChanged,
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

    TilesetDocument *tilesetDocument() const
    {
        return static_cast<TilesetDocument*>(mDocument);
    }

    Tileset *tileset() const
    {
        return tilesetDocument()->tileset().data();
    }

    Property *mNameProperty;
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

class MapObjectProperties : public ObjectProperties
{
    Q_OBJECT

public:
    MapObjectProperties(MapDocument *document, MapObject *object, ValueTypeEditorFactory *editorFactory, QObject *parent = nullptr)
        : ObjectProperties(document, object, parent)
    {
        mIdProperty = new IntProperty(
                    tr("ID"),
                    [this]() { return mapObject()->id(); });
        mIdProperty->setEnabled(false);

        mTemplateProperty = new UrlProperty(
                    tr("Template"),
                    [this]() {
                        if (auto objectTemplate = mapObject()->objectTemplate())
                            return QUrl::fromLocalFile(objectTemplate->fileName());
                        return QUrl();
                    });
        mTemplateProperty->setEnabled(false);

        mNameProperty = editorFactory->createProperty(
                    tr("Name"),
                    [this]() {
                        return mapObject()->name();
                    },
                    [this](const QVariant &value) {
                        changeMapObject(MapObject::NameProperty, value);
                    });

        mVisibleProperty = editorFactory->createProperty(
                    tr("Visible"),
                    [this]() {
                        return mapObject()->isVisible();
                    },
                    [this](const QVariant &value) {
                        changeMapObject(MapObject::VisibleProperty, value);
                    });

        mPositionProperty = editorFactory->createProperty(
                    tr("Position"),
                    [this]() {
                        return mapObject()->position();
                    },
                    [this](const QVariant &value) {
                        changeMapObject(MapObject::PositionProperty, value);
                    });

        mSizeProperty = editorFactory->createProperty(
                    tr("Size"),
                    [this]() {
                        return mapObject()->size();
                    },
                    [this](const QVariant &value) {
                        changeMapObject(MapObject::SizeProperty, value);
                    });

        mRotationProperty = new FloatProperty(
                    tr("Rotation"),
                    [this]() {
                        return mapObject()->rotation();
                    },
                    [this](const qreal &value) {
                        changeMapObject(MapObject::RotationProperty, value);
                    },
                    this);
        mRotationProperty->setSuffix(QStringLiteral("°"));

        // todo: make this a custom widget with "Horizontal" and "Vertical" checkboxes
        mFlippingProperty = editorFactory->createProperty(
                    tr("Flipping"),
                    [this]() {
                        return mapObject()->cell().flags();
                    },
                    [this](const QVariant &value) {
                        const int flippingFlags = value.toInt();

                        MapObjectCell mapObjectCell;
                        mapObjectCell.object = mapObject();
                        mapObjectCell.cell = mapObject()->cell();
                        mapObjectCell.cell.setFlippedHorizontally(flippingFlags & 1);
                        mapObjectCell.cell.setFlippedVertically(flippingFlags & 2);

                        auto command = new ChangeMapObjectCells(mDocument, { mapObjectCell });

                        command->setText(QCoreApplication::translate("Undo Commands",
                                                                     "Flip %n Object(s)",
                                                                     nullptr,
                                                                     mapDocument()->selectedObjects().size()));
                        push(command);
                    });

        mTextProperty = editorFactory->createProperty(
                    tr("Text"),
                    [this]() {
                        return mapObject()->textData().text;
                    },
                    [this](const QVariant &value) {
                        changeMapObject(MapObject::TextProperty, value);
                    });

        mTextAlignmentProperty = editorFactory->createProperty(
                    tr("Alignment"),
                    [this]() {
                        return QVariant::fromValue(mapObject()->textData().alignment);
                    },
                    [this](const QVariant &value) {
                        changeMapObject(MapObject::TextAlignmentProperty, value);
                    });

        mTextFontProperty = editorFactory->createProperty(
                    tr("Font"),
                    [this]() {
                        return mapObject()->textData().font;
                    },
                    [this](const QVariant &value) {
                        changeMapObject(MapObject::TextFontProperty, value);
                    });

        mTextWordWrapProperty = editorFactory->createProperty(
                    tr("Word Wrap"),
                    [this]() {
                        return mapObject()->textData().wordWrap;
                    },
                    [this](const QVariant &value) {
                        changeMapObject(MapObject::TextWordWrapProperty, value);
                    });

        mTextColorProperty = editorFactory->createProperty(
                    tr("Text Color"),
                    [this]() {
                        return mapObject()->textData().color;
                    },
                    [this](const QVariant &value) {
                        changeMapObject(MapObject::TextColorProperty, value);
                    });

        connect(document, &Document::changed,
                this, &MapObjectProperties::onChanged);

        updateEnabledState();
    }

    void populateEditor(VariantEditor *editor) override
    {
        editor->addHeader(tr("Object"));
        editor->addProperty(mIdProperty);
        editor->addProperty(mTemplateProperty);
        editor->addProperty(mNameProperty);
        editor->addProperty(mClassProperty);
        editor->addSeparator();

        if (mapDocument()->allowHidingObjects())
            editor->addProperty(mVisibleProperty);

        editor->addProperty(mPositionProperty);

        if (mapObject()->hasDimensions())
            editor->addProperty(mSizeProperty);

        if (mapObject()->canRotate())
            editor->addProperty(mRotationProperty);

        if (mapObject()->isTileObject()) {
            editor->addSeparator();
            editor->addProperty(mFlippingProperty);
        }

        if (mapObject()->shape() == MapObject::Text) {
            editor->addSeparator();
            editor->addProperty(mTextProperty);
            editor->addProperty(mTextAlignmentProperty);
            editor->addProperty(mTextFontProperty);
            editor->addProperty(mTextWordWrapProperty);
            editor->addProperty(mTextColorProperty);
        }
    }

private:
    void onChanged(const ChangeEvent &event)
    {
        if (event.type != ChangeEvent::MapObjectsChanged)
            return;

        const auto &change = static_cast<const MapObjectsChangeEvent&>(event);
        if (!change.mapObjects.contains(mapObject()))
            return;

        if (change.properties & MapObject::NameProperty)
            emit mNameProperty->valueChanged();
        if (change.properties & MapObject::VisibleProperty)
            emit mVisibleProperty->valueChanged();
        if (change.properties & MapObject::PositionProperty)
            emit mPositionProperty->valueChanged();
        if (change.properties & MapObject::SizeProperty)
            emit mSizeProperty->valueChanged();
        if (change.properties & MapObject::RotationProperty)
            emit mRotationProperty->valueChanged();
        if (change.properties & MapObject::CellProperty)
            emit mFlippingProperty->valueChanged();
        if (change.properties & MapObject::TextProperty)
            emit mTextProperty->valueChanged();
        if (change.properties & MapObject::TextFontProperty)
            emit mTextFontProperty->valueChanged();
        if (change.properties & MapObject::TextAlignmentProperty)
            emit mTextAlignmentProperty->valueChanged();
        if (change.properties & MapObject::TextWordWrapProperty)
            emit mTextWordWrapProperty->valueChanged();
        if (change.properties & MapObject::TextColorProperty)
            emit mTextColorProperty->valueChanged();
    }

    void updateEnabledState()
    {
        mVisibleProperty->setEnabled(mapDocument()->allowHidingObjects());
        mSizeProperty->setEnabled(mapObject()->hasDimensions());
        mRotationProperty->setEnabled(mapObject()->canRotate());
        mFlippingProperty->setEnabled(mapObject()->isTileObject());

        const bool isText = mapObject()->shape() == MapObject::Text;
        mTextProperty->setEnabled(isText);
        mTextAlignmentProperty->setEnabled(isText);
        mTextFontProperty->setEnabled(isText);
        mTextWordWrapProperty->setEnabled(isText);
        mTextColorProperty->setEnabled(isText);
    }

    MapDocument *mapDocument() const
    {
        return static_cast<MapDocument*>(mDocument);
    }

    MapObject *mapObject() const
    {
        return static_cast<MapObject*>(mObject);
    }

    void changeMapObject(MapObject::Property property, const QVariant &value)
    {
        push(new ChangeMapObject(mapDocument(), mapObject(), property, value));
    }

    Property *mIdProperty;
    Property *mTemplateProperty;
    Property *mNameProperty;
    Property *mVisibleProperty;
    Property *mPositionProperty;
    Property *mSizeProperty;
    FloatProperty *mRotationProperty;

    // for tile objects
    Property *mFlippingProperty;

    // for text objects
    Property *mTextProperty;
    Property *mTextAlignmentProperty;
    Property *mTextFontProperty;
    Property *mTextWordWrapProperty;
    Property *mTextColorProperty;
};

class TileProperties : public ObjectProperties
{
    Q_OBJECT

public:
    TileProperties(Document *document, Tile *object, ValueTypeEditorFactory *editorFactory, QObject *parent = nullptr)
        : ObjectProperties(document, object, parent)
    {
        mIdProperty = editorFactory->createProperty(
                    tr("ID"),
                    [this]() { return tile()->id(); },
                    [](const QVariant &) {});
        mIdProperty->setEnabled(false);

        // todo: apply readableImageFormatsFilter
        mImageProperty = editorFactory->createProperty(
                    tr("Image"),
                    [this]() { return tile()->imageSource(); },
                    [this](const QVariant &value) {
                        push(new ChangeTileImageSource(tilesetDocument(),
                                                       tile(),
                                                       value.toUrl()));
                    });

        mRectangleProperty = editorFactory->createProperty(
                    tr("Rectangle"),
                    [this]() { return tile()->imageRect(); },
                    [this](const QVariant &value) {
                        push(new ChangeTileImageRect(tilesetDocument(),
                                                     { tile() },
                                                     { value.toRect() }));
                    });

        // todo: minimum value should be 0
        mProbabilityProperty = editorFactory->createProperty(
                    tr("Probability"),
                    [this]() { return tile()->probability(); },
                    [this](const QVariant &value) {
                        push(new ChangeTileProbability(tilesetDocument(),
                                                       tilesetDocument()->selectedTiles(),
                                                       value.toReal()));
                    });
        mProbabilityProperty->setToolTip(tr("Relative chance this tile will be picked"));

        // annoying... maybe we should somehow always have the relevant TilesetDocument
        if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
            connect(tilesetDocument, &TilesetDocument::tileImageSourceChanged,
                    this, &TileProperties::tileImageSourceChanged);

            connect(tilesetDocument, &TilesetDocument::tileProbabilityChanged,
                    this, &TileProperties::tileProbabilityChanged);
        } else if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
            connect(mapDocument, &MapDocument::tileImageSourceChanged,
                    this, &TileProperties::tileImageSourceChanged);

            connect(mapDocument, &MapDocument::tileProbabilityChanged,
                    this, &TileProperties::tileProbabilityChanged);
        }

        updateEnabledState();
    }

    void populateEditor(VariantEditor *editor) override
    {
        editor->addHeader(tr("Tile"));
        editor->addProperty(mIdProperty);
        editor->addProperty(mClassProperty);
        editor->addSeparator();

        if (!tile()->imageSource().isEmpty())
            editor->addProperty(mImageProperty);

        editor->addProperty(mRectangleProperty);
        editor->addProperty(mProbabilityProperty);
    }

private:
    void tileImageSourceChanged(Tile *tile)
    {
        if (tile != this->tile())
            return;
        emit mImageProperty->valueChanged();
        emit mRectangleProperty->valueChanged();
    }

    void tileProbabilityChanged(Tile *tile)
    {
        if (tile != this->tile())
            return;
        emit mProbabilityProperty->valueChanged();
    }

    void updateEnabledState()
    {
        const bool hasTilesetDocument = tilesetDocument();
        const auto isCollection = tile()->tileset()->isCollection();
        mClassProperty->setEnabled(hasTilesetDocument);
        mImageProperty->setEnabled(hasTilesetDocument && isCollection);
        mRectangleProperty->setEnabled(hasTilesetDocument && isCollection);
        mProbabilityProperty->setEnabled(hasTilesetDocument);
    }

    TilesetDocument *tilesetDocument() const
    {
        return qobject_cast<TilesetDocument*>(mDocument);
    }

    Tile *tile() const
    {
        return static_cast<Tile*>(mObject);
    }

    Property *mIdProperty;
    Property *mImageProperty;
    Property *mRectangleProperty;
    Property *mProbabilityProperty;
};

class WangSetProperties : public ObjectProperties
{
    Q_OBJECT

public:
    WangSetProperties(Document *document, WangSet *object,
                      ValueTypeEditorFactory *editorFactory, QObject *parent = nullptr)
        : ObjectProperties(document, object, parent)
    {
        mNameProperty = editorFactory->createProperty(
                    tr("Name"),
                    [this]() { return wangSet()->name(); },
                    [this](const QVariant &value) {
                        push(new RenameWangSet(tilesetDocument(), wangSet(), value.toString()));
                    });

        mTypeProperty = editorFactory->createProperty(
                    tr("Type"),
                    [this]() { return QVariant::fromValue(wangSet()->type()); },
                    [this](const QVariant &value) {
                        push(new ChangeWangSetType(tilesetDocument(), wangSet(), static_cast<WangSet::Type>(value.toInt())));
                    });

        // todo: keep between 0 and WangId::MAX_COLOR_COUNT
        mColorCountProperty = editorFactory->createProperty(
                    tr("Color Count"),
                    [this]() { return wangSet()->colorCount(); },
                    [this](const QVariant &value) {
                        push(new ChangeWangSetColorCount(tilesetDocument(),
                                                         wangSet(),
                                                         value.toInt()));
                    });

        connect(document, &Document::changed,
                this, &WangSetProperties::onChanged);

        updateEnabledState();
    }

    void populateEditor(VariantEditor *editor) override
    {
        editor->addHeader(tr("Terrain Set"));
        editor->addProperty(mNameProperty);
        editor->addProperty(mClassProperty);
        editor->addSeparator();
        editor->addProperty(mTypeProperty);
        editor->addProperty(mColorCountProperty);
    }

private:
    void onChanged(const ChangeEvent &event)
    {
        if (event.type != ChangeEvent::WangSetChanged)
            return;

        const auto &wangSetChange = static_cast<const WangSetChangeEvent&>(event);
        if (wangSetChange.wangSet != wangSet())
            return;

        switch (wangSetChange.property) {
        case WangSetChangeEvent::NameProperty:
            emit mNameProperty->valueChanged();
            break;
        case WangSetChangeEvent::TypeProperty:
            emit mTypeProperty->valueChanged();
            break;
        case WangSetChangeEvent::ImageProperty:
            break;
        case WangSetChangeEvent::ColorCountProperty:
            emit mColorCountProperty->valueChanged();
            break;
        }
    }

    void updateEnabledState()
    {
        const bool hasTilesetDocument = tilesetDocument();
        mNameProperty->setEnabled(hasTilesetDocument);
        mTypeProperty->setEnabled(hasTilesetDocument);
        mColorCountProperty->setEnabled(hasTilesetDocument);
    }

    TilesetDocument *tilesetDocument() const
    {
        return qobject_cast<TilesetDocument*>(mDocument);
    }

    WangSet *wangSet()
    {
        return static_cast<WangSet*>(mObject);
    }

    Property *mNameProperty;
    Property *mTypeProperty;
    Property *mColorCountProperty;
};

class WangColorProperties : public ObjectProperties
{
    Q_OBJECT

public:
    WangColorProperties(Document *document, WangColor *object,
                        ValueTypeEditorFactory *editorFactory, QObject *parent = nullptr)
        : ObjectProperties(document, object, parent)
    {
        mNameProperty = editorFactory->createProperty(
                    tr("Name"),
                    [this]() { return wangColor()->name(); },
                    [this](const QVariant &value) {
                        push(new ChangeWangColorName(tilesetDocument(), wangColor(), value.toString()));
                    });

        mColorProperty = editorFactory->createProperty(
                    tr("Color"),
                    [this]() { return wangColor()->color(); },
                    [this](const QVariant &value) {
                        push(new ChangeWangColorColor(tilesetDocument(), wangColor(), value.value<QColor>()));
                    });

        // todo: set 0.01 as minimum
        mProbabilityProperty = editorFactory->createProperty(
                    tr("Probability"),
                    [this]() { return wangColor()->probability(); },
                    [this](const QVariant &value) {
                        push(new ChangeWangColorProbability(tilesetDocument(), wangColor(), value.toReal()));
                    });

        connect(document, &Document::changed,
                this, &WangColorProperties::onChanged);

        updateEnabledState();
    }

    void populateEditor(VariantEditor *editor) override
    {
        editor->addHeader(tr("Terrain"));
        editor->addProperty(mNameProperty);
        editor->addProperty(mClassProperty);
        editor->addSeparator();
        editor->addProperty(mColorProperty);
        editor->addProperty(mProbabilityProperty);
    }

private:
    void onChanged(const ChangeEvent &event)
    {
        if (event.type != ChangeEvent::WangColorChanged)
            return;

        const auto &wangColorChange = static_cast<const WangColorChangeEvent&>(event);
        if (wangColorChange.wangColor != wangColor())
            return;

        switch (wangColorChange.property) {
        case WangColorChangeEvent::NameProperty:
            emit mNameProperty->valueChanged();
            break;
        case WangColorChangeEvent::ColorProperty:
            emit mColorProperty->valueChanged();
            break;
        case WangColorChangeEvent::ImageProperty:
            break;
        case WangColorChangeEvent::ProbabilityProperty:
            emit mProbabilityProperty->valueChanged();
            break;
        }
    }

    void updateEnabledState()
    {
        const bool hasTilesetDocument = tilesetDocument();
        mNameProperty->setEnabled(hasTilesetDocument);
        mClassProperty->setEnabled(hasTilesetDocument);
        mColorProperty->setEnabled(hasTilesetDocument);
        mProbabilityProperty->setEnabled(hasTilesetDocument);
    }

    TilesetDocument *tilesetDocument() const
    {
        return qobject_cast<TilesetDocument*>(mDocument);
    }

    WangColor *wangColor()
    {
        return static_cast<WangColor*>(mObject);
    }

    Property *mNameProperty;
    Property *mColorProperty;
    Property *mProbabilityProperty;
};


void PropertiesWidget::currentObjectChanged(Object *object)
{
    mPropertyBrowser->clear();

    delete mPropertiesObject;
    mPropertiesObject = nullptr;

    if (object) {
        switch (object->typeId()) {
        case Object::LayerType: {
            auto mapDocument = static_cast<MapDocument*>(mDocument);

            switch (static_cast<Layer*>(object)->layerType()) {
            case Layer::ImageLayerType:
                mPropertiesObject = new ImageLayerProperties(mapDocument,
                                                             static_cast<ImageLayer*>(object),
                                                             mDefaultEditorFactory.get(), this);
                break;
            case Layer::ObjectGroupType:
                mPropertiesObject = new ObjectGroupProperties(mapDocument,
                                                              static_cast<ObjectGroup*>(object),
                                                              mDefaultEditorFactory.get(), this);
                break;
            case Layer::TileLayerType:
            case Layer::GroupLayerType:
                mPropertiesObject = new LayerProperties(mapDocument,
                                                        static_cast<ImageLayer*>(object),
                                                        mDefaultEditorFactory.get(), this);
                break;
            }
            break;
        }
        case Object::MapObjectType:
            mPropertiesObject = new MapObjectProperties(static_cast<MapDocument*>(mDocument),
                                                        static_cast<MapObject*>(object), mDefaultEditorFactory.get(), this);
            break;
        case Object::MapType:
            mPropertiesObject = new MapProperties(static_cast<MapDocument*>(mDocument),
                                                  mDefaultEditorFactory.get(), this);
            break;
        case Object::TilesetType:
            mPropertiesObject = new TilesetProperties(static_cast<TilesetDocument*>(mDocument),
                                                      mDefaultEditorFactory.get(), this);
            break;
        case Object::TileType:
            mPropertiesObject = new TileProperties(mDocument,
                                                   static_cast<Tile*>(object),
                                                   mDefaultEditorFactory.get(), this);
            break;
        case Object::WangSetType:
            mPropertiesObject = new WangSetProperties(mDocument,
                                                      static_cast<WangSet*>(object),
                                                      mDefaultEditorFactory.get(), this);
            break;
        case Object::WangColorType:
            mPropertiesObject = new WangColorProperties(mDocument,
                                                        static_cast<WangColor*>(object),
                                                        mDefaultEditorFactory.get(), this);
            break;
        case Object::ProjectType:
        case Object::WorldType:
            // these types are currently not handled by the Properties dock
            break;
        }
    }

    if (mPropertiesObject)
        mPropertiesObject->populateEditor(mPropertyBrowser);

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

    registerEditorFactory(qMetaTypeId<ObjectGroup::DrawOrder>(),
                          std::make_unique<EnumEditorFactory>(
                              QStringList {
                                  tr("Top Down"),
                                  tr("Index Order"),
                              }));

    auto wangSetTypeEditorFactory = std::make_unique<EnumEditorFactory>(
                QStringList {
                    tr("Corner"),
                    tr("Edge"),
                    tr("Mixed"),
                });

    QMap<int, QIcon> mWangSetIcons;
    mWangSetIcons.insert(WangSet::Corner, wangSetIcon(WangSet::Corner));
    mWangSetIcons.insert(WangSet::Edge, wangSetIcon(WangSet::Edge));
    mWangSetIcons.insert(WangSet::Mixed, wangSetIcon(WangSet::Mixed));
    wangSetTypeEditorFactory->setEnumIcons(mWangSetIcons);

    registerEditorFactory(qMetaTypeId<WangSet::Type>(),
                          std::move(wangSetTypeEditorFactory));
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
