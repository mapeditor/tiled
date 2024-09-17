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
#include "objectrefedit.h"
#include "objecttemplate.h"
#include "preferences.h"
#include "propertybrowser.h"
#include "tilesetchanges.h"
#include "tilesetdocument.h"
#include "utils.h"
#include "varianteditor.h"
#include "variantpropertymanager.h"
#include "wangoverlay.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QEvent>
#include <QFileInfo>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QUndoStack>
#include <QVBoxLayout>

namespace Tiled {

template<> EnumData enumData<Alignment>()
{
    return {{
        QCoreApplication::translate("Alignment", "Unspecified"),
        QCoreApplication::translate("Alignment", "Top Left"),
        QCoreApplication::translate("Alignment", "Top"),
        QCoreApplication::translate("Alignment", "Top Right"),
        QCoreApplication::translate("Alignment", "Left"),
        QCoreApplication::translate("Alignment", "Center"),
        QCoreApplication::translate("Alignment", "Right"),
        QCoreApplication::translate("Alignment", "Bottom Left"),
        QCoreApplication::translate("Alignment", "Bottom"),
        QCoreApplication::translate("Alignment", "Bottom Right")
    }};
}

template<> EnumData enumData<Map::Orientation>()
{
    // We leave out the "Unknown" orientation, because it shouldn't occur here
    return {{
        QCoreApplication::translate("Tiled::NewMapDialog", "Orthogonal"),
        QCoreApplication::translate("Tiled::NewMapDialog", "Isometric"),
        QCoreApplication::translate("Tiled::NewMapDialog", "Isometric (Staggered)"),
        QCoreApplication::translate("Tiled::NewMapDialog", "Hexagonal (Staggered)")
    }, {
        Map::Orthogonal,
        Map::Isometric,
        Map::Staggered,
        Map::Hexagonal,
    }};
}

template<> EnumData enumData<Map::StaggerAxis>()
{
    return {{
        QCoreApplication::translate("StaggerAxis", "X"),
        QCoreApplication::translate("StaggerAxis", "Y")
    }};
}

template<> EnumData enumData<Map::StaggerIndex>()
{
    return {{
        QCoreApplication::translate("StaggerIndex", "Odd"),
        QCoreApplication::translate("StaggerIndex", "Even")
    }};
}

template<> EnumData enumData<Map::RenderOrder>()
{
    return {{
        QCoreApplication::translate("RenderOrder", "Right Down"),
        QCoreApplication::translate("RenderOrder", "Right Up"),
        QCoreApplication::translate("RenderOrder", "Left Down"),
        QCoreApplication::translate("RenderOrder", "Left Up")
    }};
}

template<> EnumData enumData<Map::LayerDataFormat>()
{
    QStringList names {
        QCoreApplication::translate("PreferencesDialog", "XML (deprecated)"),
        QCoreApplication::translate("PreferencesDialog", "Base64 (uncompressed)"),
        QCoreApplication::translate("PreferencesDialog", "Base64 (gzip compressed)"),
        QCoreApplication::translate("PreferencesDialog", "Base64 (zlib compressed)"),
    };
    QList<int> values {
        Map::XML,
        Map::Base64,
        Map::Base64Gzip,
        Map::Base64Zlib,
    };

    if (compressionSupported(Zstandard)) {
        names.append(QCoreApplication::translate("PreferencesDialog", "Base64 (Zstandard compressed)"));
        values.append(Map::Base64Zstandard);
    }

    names.append(QCoreApplication::translate("PreferencesDialog", "CSV"));
    values.append(Map::CSV);

    return { names, values };
}

template<> EnumData enumData<Tileset::Orientation>()
{
    return {{
        QCoreApplication::translate("Tileset", "Orthogonal"),
        QCoreApplication::translate("Tileset", "Isometric"),
    }};
}

template<> EnumData enumData<Tileset::TileRenderSize>()
{
    return {{
        QCoreApplication::translate("Tileset", "Tile Size"),
        QCoreApplication::translate("Tileset", "Map Grid Size"),
    }};
}

template<> EnumData enumData<Tileset::FillMode>()
{
    return {{
        QCoreApplication::translate("Tileset", "Stretch"),
        QCoreApplication::translate("Tileset", "Preserve Aspect Ratio"),
    }};
}

template<> EnumData enumData<ObjectGroup::DrawOrder>()
{
    return {{
        QCoreApplication::translate("ObjectGroup", "Top Down"),
        QCoreApplication::translate("ObjectGroup", "Index Order"),
    }};
}

template<> EnumData enumData<WangSet::Type>()
{
    const QStringList names {
        QCoreApplication::translate("WangSet", "Corner"),
        QCoreApplication::translate("WangSet", "Edge"),
        QCoreApplication::translate("WangSet", "Mixed"),
    };

    QMap<int, QIcon> icons;
    icons.insert(WangSet::Corner, wangSetIcon(WangSet::Corner));
    icons.insert(WangSet::Edge, wangSetIcon(WangSet::Edge));
    icons.insert(WangSet::Mixed, wangSetIcon(WangSet::Mixed));

    return { names, {}, icons };
}


class ObjectRefProperty : public PropertyTemplate<DisplayObjectRef>
{
    Q_OBJECT

public:
    using PropertyTemplate::PropertyTemplate;

    QWidget *createEditor(QWidget *parent) override
    {
        auto editor = new ObjectRefEdit(parent);
        auto syncEditor = [this, editor] {
            const QSignalBlocker blocker(editor);
            editor->setValue(value());
        };
        syncEditor();
        connect(this, &Property::valueChanged, editor, syncEditor);
        connect(editor, &ObjectRefEdit::valueChanged,
                this, [this](const DisplayObjectRef &value) {
            setValue(value);
        });
        return editor;
    }
};


class ImageLayerRepeatProperty : public PropertyTemplate<ImageLayer::RepetitionFlags>
{
    Q_OBJECT

public:
    using PropertyTemplate::PropertyTemplate;

    QWidget *createEditor(QWidget *parent) override
    {
        auto editor = new QWidget(parent);
        auto layout = new QHBoxLayout(editor);
        auto repeatX = new QCheckBox(tr("X"), editor);
        auto repeatY = new QCheckBox(tr("Y"), editor);
        layout->setContentsMargins(QMargins());
        layout->addWidget(repeatX);
        layout->addWidget(repeatY);

        auto syncEditor = [=] {
            const QSignalBlocker xBlocker(repeatX);
            const QSignalBlocker yBlocker(repeatY);
            const auto v = value();
            repeatX->setChecked(v & ImageLayer::RepeatX);
            repeatY->setChecked(v & ImageLayer::RepeatY);
        };
        auto syncProperty = [=] {
            ImageLayer::RepetitionFlags v;
            if (repeatX->isChecked())
                v |= ImageLayer::RepeatX;
            if (repeatY->isChecked())
                v |= ImageLayer::RepeatY;
            setValue(v);
        };

        syncEditor();

        connect(this, &Property::valueChanged, editor, syncEditor);
        connect(repeatX, &QCheckBox::toggled, this, syncProperty);
        connect(repeatY, &QCheckBox::toggled, this, syncProperty);
        return editor;
    }
};


class TransformationFlagsProperty : public PropertyTemplate<Tileset::TransformationFlags>
{
    Q_OBJECT

public:
    using PropertyTemplate::PropertyTemplate;

    QWidget *createEditor(QWidget *parent) override
    {
        QIcon flipHorizontalIcon(QLatin1String(":images/24/flip-horizontal.png"));
        QIcon flipVerticalIcon(QLatin1String(":images/24/flip-vertical.png"));
        QIcon rotateRightIcon(QLatin1String(":images/24/rotate-right.png"));

        flipHorizontalIcon.addFile(QLatin1String(":images/32/flip-horizontal.png"));
        flipVerticalIcon.addFile(QLatin1String(":images/32/flip-vertical.png"));
        rotateRightIcon.addFile(QLatin1String(":images/32/rotate-right.png"));

        auto editor = new QWidget(parent);

        auto flipHorizontally = new QToolButton(editor);
        flipHorizontally->setToolTip(tr("Flip Horizontally"));
        flipHorizontally->setIcon(flipHorizontalIcon);
        flipHorizontally->setCheckable(true);

        auto flipVertically = new QToolButton(editor);
        flipVertically->setToolTip(tr("Flip Vertically"));
        flipVertically->setIcon(flipVerticalIcon);
        flipVertically->setCheckable(true);

        auto rotate = new QToolButton(editor);
        rotate->setToolTip(tr("Rotate"));
        rotate->setIcon(rotateRightIcon);
        rotate->setCheckable(true);

        auto preferUntransformed = new QCheckBox(tr("Prefer Untransformed"), editor);

        auto horizontalLayout = new QHBoxLayout;
        horizontalLayout->addWidget(flipHorizontally);
        horizontalLayout->addWidget(flipVertically);
        horizontalLayout->addWidget(rotate);
        horizontalLayout->addStretch();

        auto verticalLayout = new QVBoxLayout(editor);
        verticalLayout->setContentsMargins(QMargins());
        verticalLayout->setSpacing(Utils::dpiScaled(4));
        verticalLayout->addLayout(horizontalLayout);
        verticalLayout->addWidget(preferUntransformed);

        auto syncEditor = [=] {
            const QSignalBlocker horizontalBlocker(flipHorizontally);
            const QSignalBlocker verticalBlocker(flipVertically);
            const QSignalBlocker rotateBlocker(rotate);
            const QSignalBlocker preferUntransformedBlocker(preferUntransformed);
            const auto v = value();
            flipHorizontally->setChecked(v & Tileset::AllowFlipHorizontally);
            flipVertically->setChecked(v & Tileset::AllowFlipVertically);
            rotate->setChecked(v & Tileset::AllowRotate);
            preferUntransformed->setChecked(v & Tileset::PreferUntransformed);
        };
        auto syncProperty = [=] {
            Tileset::TransformationFlags v;
            if (flipHorizontally->isChecked())
                v |= Tileset::AllowFlipHorizontally;
            if (flipVertically->isChecked())
                v |= Tileset::AllowFlipVertically;
            if (rotate->isChecked())
                v |= Tileset::AllowRotate;
            if (preferUntransformed->isChecked())
                v |= Tileset::PreferUntransformed;
            setValue(v);
        };

        syncEditor();

        connect(this, &Property::valueChanged, editor, syncEditor);
        connect(flipHorizontally, &QAbstractButton::toggled, this, syncProperty);
        connect(flipVertically, &QAbstractButton::toggled, this, syncProperty);
        connect(rotate, &QAbstractButton::toggled, this, syncProperty);
        connect(preferUntransformed, &QAbstractButton::toggled, this, syncProperty);
        return editor;
    }
};


PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QWidget{parent}
    , mScrollArea(new QScrollArea(this))
{
    auto scrollWidget = new QWidget(mScrollArea);
    scrollWidget->setBackgroundRole(QPalette::AlternateBase);

    auto verticalLayout = new QVBoxLayout(scrollWidget);
    mPropertyBrowser = new VariantEditor(scrollWidget);
    verticalLayout->addWidget(mPropertyBrowser);
    verticalLayout->addStretch();
    verticalLayout->setContentsMargins(0, 0, 0, Utils::dpiScaled(4));

    mScrollArea->setWidget(scrollWidget);
    mScrollArea->setWidgetResizable(true);

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
    layout->addWidget(mScrollArea);
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
            editor->setCurrentText(value());
        };
        syncEditor();
        connect(this, &Property::valueChanged, editor, syncEditor);
        connect(editor, &QComboBox::currentTextChanged, this, &StringProperty::setValue);
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
    MapProperties(MapDocument *document, QObject *parent = nullptr)
        : ObjectProperties(document, document->map(), parent)
    {
        mOrientationProperty = new EnumProperty<Map::Orientation>(
                    tr("Orientation"),
                    [this] {
                        return map()->orientation();
                    },
                    [this](Map::Orientation value) {
                        push(new ChangeMapProperty(mapDocument(), value));
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
        mTileSizeProperty->setMinimum(1);

        mInfiniteProperty = new BoolProperty(
                    tr("Infinite"),
                    [this] {
                        return map()->infinite();
                    },
                    [this](const bool &value) {
                        push(new ChangeMapProperty(mapDocument(),
                                                   Map::InfiniteProperty,
                                                   value ? 1 : 0));
                    });

        mHexSideLengthProperty = new IntProperty(
                    tr("Hex Side Length"),
                    [this] {
                        return map()->hexSideLength();
                    },
                    [this](const QVariant &value) {
                        push(new ChangeMapProperty(mapDocument(),
                                                   Map::HexSideLengthProperty,
                                                   value.toInt()));
                    });

        mStaggerAxisProperty = new EnumProperty<Map::StaggerAxis>(
                    tr("Stagger Axis"),
                    [this] {
                        return map()->staggerAxis();
                    },
                    [this](Map::StaggerAxis value) {
                        push(new ChangeMapProperty(mapDocument(), value));
                    });

        mStaggerIndexProperty = new EnumProperty<Map::StaggerIndex>(
                    tr("Stagger Index"),
                    [this] {
                        return map()->staggerIndex();
                    },
                    [this](Map::StaggerIndex value) {
                        push(new ChangeMapProperty(mapDocument(), value));
                    });

        mParallaxOriginProperty = new PointFProperty(
                    tr("Parallax Origin"),
                    [this] {
                        return map()->parallaxOrigin();
                    },
                    [this](const QPointF &value) {
                        push(new ChangeMapProperty(mapDocument(), value));
                    });

        mLayerDataFormatProperty = new EnumProperty<Map::LayerDataFormat>(
                    tr("Layer Data Format"),
                    [this] {
                        return map()->layerDataFormat();
                    },
                    [this](Map::LayerDataFormat value) {
                        push(new ChangeMapProperty(mapDocument(), value));
                    });

        mChunkSizeProperty = new SizeProperty(
                    tr("Output Chunk Size"),
                    [this] {
                        return map()->chunkSize();
                    },
                    [this](const QSize &value) {
                        push(new ChangeMapProperty(mapDocument(), value));
                    });
        mChunkSizeProperty->setMinimum(CHUNK_SIZE_MIN);

        mRenderOrderProperty = new EnumProperty<Map::RenderOrder>(
                    tr("Tile Render Order"),
                    [this] {
                        return map()->renderOrder();
                    },
                    [this](Map::RenderOrder value) {
                        push(new ChangeMapProperty(mapDocument(), value));
                    });

        mCompressionLevelProperty = new IntProperty(
                    tr("Compression Level"),
                    [this] {
                        return map()->compressionLevel();
                    },
                    [this](const int &value) {
                        push(new ChangeMapProperty(mapDocument(), value));
                    });

        mBackgroundColorProperty = new ColorProperty(
                    tr("Background Color"),
                    [this] {
                        return map()->backgroundColor();
                    },
                    [this](const QColor &value) {
                        push(new ChangeMapProperty(mapDocument(), value));
                    });

        mMapProperties = new GroupProperty(tr("Map"));
        mMapProperties->addProperty(mClassProperty);
        mMapProperties->addSeparator();
        mMapProperties->addProperty(mOrientationProperty);
        mMapProperties->addProperty(mSizeProperty);
        mMapProperties->addProperty(mInfiniteProperty);
        mMapProperties->addProperty(mTileSizeProperty);
        mMapProperties->addProperty(mHexSideLengthProperty);
        mMapProperties->addProperty(mStaggerAxisProperty);
        mMapProperties->addProperty(mStaggerIndexProperty);
        mMapProperties->addSeparator();
        mMapProperties->addProperty(mParallaxOriginProperty);
        mMapProperties->addSeparator();
        mMapProperties->addProperty(mLayerDataFormatProperty);
        mMapProperties->addProperty(mChunkSizeProperty);
        mMapProperties->addProperty(mCompressionLevelProperty);
        mMapProperties->addSeparator();
        mMapProperties->addProperty(mRenderOrderProperty);
        mMapProperties->addProperty(mBackgroundColorProperty);

        updateEnabledState();
        connect(document, &Document::changed,
                this, &MapProperties::onChanged);
    }

    void populateEditor(VariantEditor *editor) override
    {
        editor->addProperty(mMapProperties);
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

    GroupProperty *mMapProperties;
    Property *mOrientationProperty;
    Property *mSizeProperty;
    SizeProperty *mTileSizeProperty;
    Property *mInfiniteProperty;
    Property *mHexSideLengthProperty;
    Property *mStaggerAxisProperty;
    Property *mStaggerIndexProperty;
    Property *mParallaxOriginProperty;
    Property *mLayerDataFormatProperty;
    SizeProperty *mChunkSizeProperty;
    Property *mRenderOrderProperty;
    Property *mCompressionLevelProperty;
    Property *mBackgroundColorProperty;
};

class LayerProperties : public ObjectProperties
{
    Q_OBJECT

public:
    LayerProperties(MapDocument *document, Layer *object, QObject *parent = nullptr)
        : ObjectProperties(document, object, parent)
    {
        // todo: would be nicer to avoid the SpinBox and use a custom widget
        // might also be nice to embed this in the header instead of using a property
        mIdProperty = new IntProperty(
                    tr("ID"),
                    [this] { return layer()->id(); });
        mIdProperty->setEnabled(false);

        // todo: the below should be able to apply to all selected layers

        mNameProperty = new StringProperty(
                    tr("Name"),
                    [this] { return layer()->name(); },
                    [this](const QString &value) {
                        push(new SetLayerName(mapDocument(), { layer() }, value));
                    });

        mVisibleProperty = new BoolProperty(
                    tr("Visible"),
                    [this] { return layer()->isVisible(); },
                    [this](const bool &value) {
                        push(new SetLayerVisible(mapDocument(), { layer() }, value));
                    });

        mLockedProperty = new BoolProperty(
                    tr("Locked"),
                    [this] { return layer()->isLocked(); },
                    [this](const bool &value) {
                        push(new SetLayerLocked(mapDocument(), { layer() }, value));
                    });

        mOpacityProperty = new IntProperty(
                    tr("Opacity"),
                    [this] { return qRound(layer()->opacity() * 100); },
                    [this](const int &value) {
                        push(new SetLayerOpacity(mapDocument(), { layer() }, qreal(value) / 100));
                    });
        mOpacityProperty->setRange(0, 100);
        mOpacityProperty->setSuffix(tr("%"));
        mOpacityProperty->setSliderEnabled(true);

        mTintColorProperty = new ColorProperty(
                    tr("Tint Color"),
                    [this] { return layer()->tintColor(); },
                    [this](const QColor &value) {
                        push(new SetLayerTintColor(mapDocument(), { layer() }, value));
                    });

        mOffsetProperty = new PointFProperty(
                    tr("Offset"),
                    [this] { return layer()->offset(); },
                    [this](const QPointF &value) {
                        push(new SetLayerOffset(mapDocument(), { layer() }, value));
                    });

        mParallaxFactorProperty = new PointFProperty(
                    tr("Parallax Factor"),
                    [this] { return layer()->parallaxFactor(); },
                    [this](const QPointF &value) {
                        push(new SetLayerParallaxFactor(mapDocument(), { layer() }, value));
                    });
        mParallaxFactorProperty->setSingleStep(0.1);

        mLayerProperties = new GroupProperty(tr("Layer"));
        mLayerProperties->addProperty(mIdProperty);
        mLayerProperties->addProperty(mNameProperty);
        mLayerProperties->addProperty(mClassProperty);
        mLayerProperties->addSeparator();
        mLayerProperties->addProperty(mVisibleProperty);
        mLayerProperties->addProperty(mLockedProperty);
        mLayerProperties->addProperty(mOpacityProperty);
        mLayerProperties->addProperty(mTintColorProperty);
        mLayerProperties->addProperty(mOffsetProperty);
        mLayerProperties->addProperty(mParallaxFactorProperty);

        connect(document, &Document::changed,
                this, &LayerProperties::onChanged);
    }

    void populateEditor(VariantEditor *editor) override
    {
        editor->addProperty(mLayerProperties);
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

    GroupProperty *mLayerProperties;
    Property *mIdProperty;
    Property *mNameProperty;
    Property *mVisibleProperty;
    Property *mLockedProperty;
    IntProperty *mOpacityProperty;
    Property *mTintColorProperty;
    Property *mOffsetProperty;
    PointFProperty *mParallaxFactorProperty;
};

class ImageLayerProperties : public LayerProperties
{
    Q_OBJECT

public:
    ImageLayerProperties(MapDocument *document, ImageLayer *object, QObject *parent = nullptr)
        : LayerProperties(document, object, parent)
    {
        mImageProperty = new UrlProperty(
                    tr("Image Source"),
                    [this] { return imageLayer()->imageSource(); },
                    [this](const QUrl &value) {
                        push(new ChangeImageLayerImageSource(mapDocument(), { imageLayer() }, value));
                    });
        mImageProperty->setFilter(Utils::readableImageFormatsFilter());

        mTransparentColorProperty = new ColorProperty(
                    tr("Transparent Color"),
                    [this] { return imageLayer()->transparentColor(); },
                    [this](const QColor &value) {
                        push(new ChangeImageLayerTransparentColor(mapDocument(), { imageLayer() }, value));
                    });

        mRepeatProperty = new ImageLayerRepeatProperty(
                    tr("Repeat"),
                    [this] { return imageLayer()->repetition(); },
                    [this](const ImageLayer::RepetitionFlags &value) {
                        const bool repeatX = value & ImageLayer::RepeatX;
                        const bool repeatY = value & ImageLayer::RepeatY;
                        if (repeatX != imageLayer()->repeatX())
                            push(new ChangeImageLayerRepeatX(mapDocument(), { imageLayer() }, repeatX));
                        if (repeatY != imageLayer()->repeatY())
                            push(new ChangeImageLayerRepeatY(mapDocument(), { imageLayer() }, repeatY));
                    });

        mImageLayerProperties = new GroupProperty(tr("Image Layer"));
        mImageLayerProperties->addProperty(mImageProperty);
        mImageLayerProperties->addProperty(mTransparentColorProperty);
        mImageLayerProperties->addSeparator();
        mImageLayerProperties->addProperty(mRepeatProperty);
    }

    void populateEditor(VariantEditor *editor) override
    {
        LayerProperties::populateEditor(editor);
        editor->addProperty(mImageLayerProperties);
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
            emit mRepeatProperty->valueChanged();
        }
    }

    ImageLayer *imageLayer() const
    {
        return static_cast<ImageLayer*>(mObject);
    }

    GroupProperty *mImageLayerProperties;
    UrlProperty *mImageProperty;
    Property *mTransparentColorProperty;
    Property *mRepeatProperty;
};

class ObjectGroupProperties : public LayerProperties
{
    Q_OBJECT

public:
    ObjectGroupProperties(MapDocument *document, ObjectGroup *object, QObject *parent = nullptr)
        : LayerProperties(document, object, parent)
    {
        mColorProperty = new ColorProperty(
                    tr("Color"),
                    [this] { return objectGroup()->color(); },
                    [this](const QColor &value) {
                        push(new ChangeObjectGroupColor(mapDocument(), { objectGroup() }, value));
                    });

        mDrawOrderProperty = new EnumProperty<ObjectGroup::DrawOrder>(
                    tr("Draw Order"),
                    [this] { return objectGroup()->drawOrder(); },
                    [this](ObjectGroup::DrawOrder value) {
                        push(new ChangeObjectGroupDrawOrder(mapDocument(), { objectGroup() }, value));
                    });

        mObjectGroupProperties = new GroupProperty(tr("Object Layer"));
        mObjectGroupProperties->addProperty(mColorProperty);
        mObjectGroupProperties->addProperty(mDrawOrderProperty);
    }

    void populateEditor(VariantEditor *editor) override
    {
        LayerProperties::populateEditor(editor);
        editor->addProperty(mObjectGroupProperties);
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

    GroupProperty *mObjectGroupProperties;
    Property *mColorProperty;
    Property *mDrawOrderProperty;
};

class TilesetProperties : public ObjectProperties
{
    Q_OBJECT

public:
    TilesetProperties(TilesetDocument *document, QObject *parent = nullptr)
        : ObjectProperties(document, document->tileset().data(), parent)
    {
        mNameProperty = new StringProperty(
                    tr("Name"),
                    [this] {
                        return tilesetDocument()->tileset()->name();
                    },
                    [this](const QString &value) {
                        push(new RenameTileset(tilesetDocument(), value));
                    });

        mObjectAlignmentProperty = new EnumProperty<Alignment>(
                    tr("Object Alignment"),
                    [this] {
                        return tileset()->objectAlignment();
                    },
                    [this](Alignment value) {
                        push(new ChangeTilesetObjectAlignment(tilesetDocument(), value));
                    });

        mTileOffsetProperty = new PointProperty(
                    tr("Drawing Offset"),
                    [this] {
                        return tileset()->tileOffset();
                    },
                    [this](const QPoint &value) {
                        push(new ChangeTilesetTileOffset(tilesetDocument(), value));
                    });

        mTileRenderSizeProperty = new EnumProperty<Tileset::TileRenderSize>(
                    tr("Tile Render Size"),
                    [this] {
                        return tileset()->tileRenderSize();
                    },
                    [this](Tileset::TileRenderSize value) {
                        push(new ChangeTilesetTileRenderSize(tilesetDocument(), value));
                    });

        mFillModeProperty = new EnumProperty<Tileset::FillMode>(
                    tr("Fill Mode"),
                    [this] {
                        return tileset()->fillMode();
                    },
                    [this](Tileset::FillMode value) {
                        push(new ChangeTilesetFillMode(tilesetDocument(), value));
                    });

        mBackgroundColorProperty = new ColorProperty(
                    tr("Background Color"),
                    [this] {
                        return tileset()->backgroundColor();
                    },
                    [this](const QColor &value) {
                        push(new ChangeTilesetBackgroundColor(tilesetDocument(), value));
                    });

        mOrientationProperty = new EnumProperty<Tileset::Orientation>(
                    tr("Orientation"),
                    [this] {
                        return tileset()->orientation();
                    },
                    [this](Tileset::Orientation value) {
                        push(new ChangeTilesetOrientation(tilesetDocument(), value));
                    });

        mGridSizeProperty = new SizeProperty(
                    tr("Grid Size"),
                    [this] {
                        return tileset()->gridSize();
                    },
                    [this](const QSize &value) {
                        push(new ChangeTilesetGridSize(tilesetDocument(), value));
                    });
        mGridSizeProperty->setMinimum(1);

        mColumnCountProperty = new IntProperty(
                    tr("Columns"),
                    [this] {
                        return tileset()->columnCount();
                    },
                    [this](const int &value) {
                        push(new ChangeTilesetColumnCount(tilesetDocument(), value));
                    });
        mColumnCountProperty->setMinimum(1);

        mAllowedTransformationsProperty = new TransformationFlagsProperty(
                    tr("Allowed Transformations"),
                    [this] {
                        return tileset()->transformationFlags();
                    },
                    [this](const Tileset::TransformationFlags &value) {
                        push(new ChangeTilesetTransformationFlags(tilesetDocument(), value));
                    });

        // todo: this needs a custom widget
        mImageProperty = new UrlProperty(
                    tr("Image"),
                    [this] {
                        return tileset()->imageSource();
                    },
                    [](const QUrl &) {
                        // push(new ChangeTilesetImage(tilesetDocument(), value.toString()));
                    });

        mTilesetProperties = new GroupProperty(tr("Tileset"));
        mTilesetProperties->addProperty(mNameProperty);
        mTilesetProperties->addProperty(mClassProperty);
        mTilesetProperties->addSeparator();
        mTilesetProperties->addProperty(mObjectAlignmentProperty);
        mTilesetProperties->addProperty(mTileOffsetProperty);
        mTilesetProperties->addProperty(mTileRenderSizeProperty);
        mTilesetProperties->addProperty(mFillModeProperty);
        mTilesetProperties->addProperty(mBackgroundColorProperty);
        mTilesetProperties->addProperty(mOrientationProperty);
        mTilesetProperties->addProperty(mGridSizeProperty);
        mTilesetProperties->addProperty(mColumnCountProperty);
        mTilesetProperties->addProperty(mAllowedTransformationsProperty);
        mTilesetProperties->addProperty(mImageProperty);

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
        editor->addProperty(mTilesetProperties);
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

    GroupProperty *mTilesetProperties;
    Property *mNameProperty;
    Property *mObjectAlignmentProperty;
    Property *mTileOffsetProperty;
    Property *mTileRenderSizeProperty;
    Property *mFillModeProperty;
    Property *mBackgroundColorProperty;
    Property *mOrientationProperty;
    SizeProperty *mGridSizeProperty;
    IntProperty *mColumnCountProperty;
    Property *mAllowedTransformationsProperty;
    Property *mImageProperty;
};

class MapObjectProperties : public ObjectProperties
{
    Q_OBJECT

public:
    MapObjectProperties(MapDocument *document, MapObject *object, QObject *parent = nullptr)
        : ObjectProperties(document, object, parent)
    {
        mIdProperty = new IntProperty(
                    tr("ID"),
                    [this] { return mapObject()->id(); });
        mIdProperty->setEnabled(false);

        mTemplateProperty = new UrlProperty(
                    tr("Template"),
                    [this] {
                        if (auto objectTemplate = mapObject()->objectTemplate())
                            return QUrl::fromLocalFile(objectTemplate->fileName());
                        return QUrl();
                    });
        mTemplateProperty->setEnabled(false);

        mNameProperty = new StringProperty(
                    tr("Name"),
                    [this] {
                        return mapObject()->name();
                    },
                    [this](const QString &value) {
                        changeMapObject(MapObject::NameProperty, value);
                    });

        mVisibleProperty = new BoolProperty(
                    tr("Visible"),
                    [this] {
                        return mapObject()->isVisible();
                    },
                    [this](const bool &value) {
                        changeMapObject(MapObject::VisibleProperty, value);
                    });

        mPositionProperty = new PointFProperty(
                    tr("Position"),
                    [this] {
                        return mapObject()->position();
                    },
                    [this](const QPointF &value) {
                        changeMapObject(MapObject::PositionProperty, value);
                    });

        mSizeProperty = new SizeFProperty(
                    tr("Size"),
                    [this] {
                        return mapObject()->size();
                    },
                    [this](const QSizeF &value) {
                        changeMapObject(MapObject::SizeProperty, value);
                    });

        mRotationProperty = new FloatProperty(
                    tr("Rotation"),
                    [this] {
                        return mapObject()->rotation();
                    },
                    [this](const qreal &value) {
                        changeMapObject(MapObject::RotationProperty, value);
                    },
                    this);
        mRotationProperty->setSuffix(QStringLiteral("°"));

        // todo: make this a custom widget with "Horizontal" and "Vertical" checkboxes
        mFlippingProperty = new IntProperty(
                    tr("Flipping"),
                    [this] {
                        return mapObject()->cell().flags();
                    },
                    [this](const int &value) {
                        const int flippingFlags = value;

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

        mTextProperty = new MultilineStringProperty(
                    tr("Text"),
                    [this] {
                        return mapObject()->textData().text;
                    },
                    [this](const QString &value) {
                        changeMapObject(MapObject::TextProperty, value);
                    });

        mTextAlignmentProperty = new QtAlignmentProperty(
                    tr("Alignment"),
                    [this] {
                        return mapObject()->textData().alignment;
                    },
                    [this](const Qt::Alignment &value) {
                        changeMapObject(MapObject::TextAlignmentProperty,
                                        QVariant::fromValue(value));
                    });

        mTextFontProperty = new FontProperty(
                    tr("Font"),
                    [this] {
                        return mapObject()->textData().font;
                    },
                    [this](const QFont &value) {
                        changeMapObject(MapObject::TextFontProperty, value);
                    });

        mTextWordWrapProperty = new BoolProperty(
                    tr("Word Wrap"),
                    [this] {
                        return mapObject()->textData().wordWrap;
                    },
                    [this](const bool &value) {
                        changeMapObject(MapObject::TextWordWrapProperty, value);
                    });

        mTextColorProperty = new ColorProperty(
                    tr("Text Color"),
                    [this] {
                        return mapObject()->textData().color;
                    },
                    [this](const QColor &value) {
                        changeMapObject(MapObject::TextColorProperty, value);
                    });

        mObjectProperties = new GroupProperty(tr("Object"));
        mObjectProperties->addProperty(mIdProperty);
        mObjectProperties->addProperty(mTemplateProperty);
        mObjectProperties->addProperty(mNameProperty);
        mObjectProperties->addProperty(mClassProperty);
        mObjectProperties->addSeparator();

        if (mapDocument()->allowHidingObjects())
            mObjectProperties->addProperty(mVisibleProperty);

        mObjectProperties->addProperty(mPositionProperty);

        if (mapObject()->hasDimensions())
            mObjectProperties->addProperty(mSizeProperty);

        if (mapObject()->canRotate())
            mObjectProperties->addProperty(mRotationProperty);

        if (mapObject()->isTileObject()) {
            mObjectProperties->addSeparator();
            mObjectProperties->addProperty(mFlippingProperty);
        }

        if (mapObject()->shape() == MapObject::Text) {
            mObjectProperties->addSeparator();
            mObjectProperties->addProperty(mTextProperty);
            mObjectProperties->addProperty(mTextAlignmentProperty);
            mObjectProperties->addProperty(mTextFontProperty);
            mObjectProperties->addProperty(mTextWordWrapProperty);
            mObjectProperties->addProperty(mTextColorProperty);
        }

        connect(document, &Document::changed,
                this, &MapObjectProperties::onChanged);

        updateEnabledState();
    }

    void populateEditor(VariantEditor *editor) override
    {
        editor->addProperty(mObjectProperties);
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

    GroupProperty *mObjectProperties;
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
    TileProperties(Document *document, Tile *object, QObject *parent = nullptr)
        : ObjectProperties(document, object, parent)
    {
        mIdProperty = new IntProperty(
                    tr("ID"),
                    [this] { return tile()->id(); });
        mIdProperty->setEnabled(false);

        mImageProperty = new UrlProperty(
                    tr("Image"),
                    [this] { return tile()->imageSource(); },
                    [this](const QUrl &value) {
                        push(new ChangeTileImageSource(tilesetDocument(),
                                                       tile(),
                                                       value));
                    });
        mImageProperty->setFilter(Utils::readableImageFormatsFilter());

        mRectangleProperty = new RectProperty(
                    tr("Rectangle"),
                    [this] { return tile()->imageRect(); },
                    [this](const QRect &value) {
                        push(new ChangeTileImageRect(tilesetDocument(),
                                                     { tile() },
                                                     { value }));
                    });
        mRectangleProperty->setConstraint(object->image().rect());

        mProbabilityProperty = new FloatProperty(
                    tr("Probability"),
                    [this] { return tile()->probability(); },
                    [this](const double &value) {
                        push(new ChangeTileProbability(tilesetDocument(),
                                                       tilesetDocument()->selectedTiles(),
                                                       value));
                    });
        mProbabilityProperty->setToolTip(tr("Relative chance this tile will be picked"));
        mProbabilityProperty->setMinimum(0.0);

        mTileProperties = new GroupProperty(tr("Tile"));
        mTileProperties->addProperty(mIdProperty);
        mTileProperties->addProperty(mClassProperty);
        mTileProperties->addSeparator();

        if (!tile()->imageSource().isEmpty())
            mTileProperties->addProperty(mImageProperty);

        mTileProperties->addProperty(mRectangleProperty);
        mTileProperties->addProperty(mProbabilityProperty);

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
        editor->addProperty(mTileProperties);
    }

private:
    void tileImageSourceChanged(Tile *tile)
    {
        if (tile != this->tile())
            return;
        mRectangleProperty->setConstraint(tile->image().rect());
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

    GroupProperty *mTileProperties;
    Property *mIdProperty;
    UrlProperty *mImageProperty;
    RectProperty *mRectangleProperty;
    FloatProperty *mProbabilityProperty;
};

class WangSetProperties : public ObjectProperties
{
    Q_OBJECT

public:
    WangSetProperties(Document *document, WangSet *object,
                      QObject *parent = nullptr)
        : ObjectProperties(document, object, parent)
    {
        mNameProperty = new StringProperty(
                    tr("Name"),
                    [this] { return wangSet()->name(); },
                    [this](const QString &value) {
                        push(new RenameWangSet(tilesetDocument(), wangSet(), value));
                    });

        mTypeProperty = new EnumProperty<WangSet::Type>(
                    tr("Type"),
                    [this] { return wangSet()->type(); },
                    [this](WangSet::Type value) {
                        push(new ChangeWangSetType(tilesetDocument(), wangSet(), value));
                    });

        mColorCountProperty = new IntProperty(
                    tr("Color Count"),
                    [this] { return wangSet()->colorCount(); },
                    [this](const int &value) {
                        push(new ChangeWangSetColorCount(tilesetDocument(),
                                                         wangSet(),
                                                         value));
                    });
        mColorCountProperty->setRange(0, WangId::MAX_COLOR_COUNT);

        mWangSetProperties = new GroupProperty(tr("Terrain Set"));
        mWangSetProperties->addProperty(mNameProperty);
        mWangSetProperties->addProperty(mClassProperty);
        mWangSetProperties->addSeparator();
        mWangSetProperties->addProperty(mTypeProperty);
        mWangSetProperties->addProperty(mColorCountProperty);

        connect(document, &Document::changed,
                this, &WangSetProperties::onChanged);

        updateEnabledState();
    }

    void populateEditor(VariantEditor *editor) override
    {
        editor->addProperty(mWangSetProperties);
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

    GroupProperty *mWangSetProperties;
    Property *mNameProperty;
    Property *mTypeProperty;
    IntProperty *mColorCountProperty;
};

class WangColorProperties : public ObjectProperties
{
    Q_OBJECT

public:
    WangColorProperties(Document *document, WangColor *object,
                        QObject *parent = nullptr)
        : ObjectProperties(document, object, parent)
    {
        mNameProperty = new StringProperty(
                    tr("Name"),
                    [this] { return wangColor()->name(); },
                    [this](const QString &value) {
                        push(new ChangeWangColorName(tilesetDocument(), wangColor(), value));
                    });

        mColorProperty = new ColorProperty(
                    tr("Color"),
                    [this] { return wangColor()->color(); },
                    [this](const QColor &value) {
                        push(new ChangeWangColorColor(tilesetDocument(), wangColor(), value));
                    });

        mProbabilityProperty = new FloatProperty(
                    tr("Probability"),
                    [this] { return wangColor()->probability(); },
                    [this](const double &value) {
                        push(new ChangeWangColorProbability(tilesetDocument(), wangColor(), value));
                    });
        mProbabilityProperty->setMinimum(0.01);

        mWangColorProperties = new GroupProperty(tr("Terrain"));
        mWangColorProperties->addProperty(mNameProperty);
        mWangColorProperties->addProperty(mClassProperty);
        mWangColorProperties->addSeparator();
        mWangColorProperties->addProperty(mColorProperty);
        mWangColorProperties->addProperty(mProbabilityProperty);

        connect(document, &Document::changed,
                this, &WangColorProperties::onChanged);

        updateEnabledState();
    }

    void populateEditor(VariantEditor *editor) override
    {
        editor->addProperty(mWangColorProperties);
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

    GroupProperty *mWangColorProperties;
    Property *mNameProperty;
    Property *mColorProperty;
    FloatProperty *mProbabilityProperty;
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
                                                             this);
                break;
            case Layer::ObjectGroupType:
                mPropertiesObject = new ObjectGroupProperties(mapDocument,
                                                              static_cast<ObjectGroup*>(object),
                                                              this);
                break;
            case Layer::TileLayerType:
            case Layer::GroupLayerType:
                mPropertiesObject = new LayerProperties(mapDocument,
                                                        static_cast<ImageLayer*>(object),
                                                        this);
                break;
            }
            break;
        }
        case Object::MapObjectType:
            mPropertiesObject = new MapObjectProperties(static_cast<MapDocument*>(mDocument),
                                                        static_cast<MapObject*>(object), this);
            break;
        case Object::MapType:
            mPropertiesObject = new MapProperties(static_cast<MapDocument*>(mDocument),
                                                  this);
            break;
        case Object::TilesetType:
            mPropertiesObject = new TilesetProperties(static_cast<TilesetDocument*>(mDocument),
                                                      this);
            break;
        case Object::TileType:
            mPropertiesObject = new TileProperties(mDocument,
                                                   static_cast<Tile*>(object),
                                                   this);
            break;
        case Object::WangSetType:
            mPropertiesObject = new WangSetProperties(mDocument,
                                                      static_cast<WangSet*>(object),
                                                      this);
            break;
        case Object::WangColorType:
            mPropertiesObject = new WangColorProperties(mDocument,
                                                        static_cast<WangColor*>(object),
                                                        this);
            break;
        case Object::ProjectType:
        case Object::WorldType:
            // these types are currently not handled by the Properties dock
            break;
        }
    }

    if (mPropertiesObject)
        mPropertiesObject->populateEditor(mPropertyBrowser);

    GroupProperty *customProperties = new GroupProperty(tr("Custom Properties"));

    QMapIterator<QString, QVariant> it(object ? object->properties() : Properties());
    PropertyFactory factory;

    while (it.hasNext()) {
        it.next();

        const auto &name = it.key();
        const auto &value = it.value();

        Property *property = nullptr;

        switch (value.userType()) {
        case QMetaType::Bool:
        case QMetaType::QColor:
        case QMetaType::Double:
        case QMetaType::Int:
        case QMetaType::QString: {
            auto get = [object, name] { return object->property(name); };
            auto set = [this, object, name] (const QVariant &value) {
                mDocument->undoStack()->push(new SetProperty(mDocument, { object }, name, value));
            };
            property = factory.createProperty(name, std::move(get), std::move(set));
            break;
        }
        default:
            if (value.userType() == filePathTypeId()) {
                auto get = [object, name] { return object->property(name).value<FilePath>().url; };
                auto set = [this, object, name](const QUrl &value) {
                    mDocument->undoStack()->push(new SetProperty(mDocument, { object }, name, QVariant::fromValue(FilePath { value })));
                };
                property = new UrlProperty(name, get, set);
            } else if (value.userType() == objectRefTypeId()) {
                auto get = [this, object, name] { return DisplayObjectRef(object->property(name).value<ObjectRef>(), static_cast<MapDocument*>(mDocument)); };
                auto set = [this, object, name](const DisplayObjectRef &value) {
                    mDocument->undoStack()->push(new SetProperty(mDocument, { object }, name, QVariant::fromValue(value.ref)));
                };
                property = new ObjectRefProperty(name, get, set);
            }
            // todo: PropertyValue (enum and class values)
            break;
        }

        if (property)
            customProperties->addProperty(property);
    }

    mPropertyBrowser->addProperty(customProperties);

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
