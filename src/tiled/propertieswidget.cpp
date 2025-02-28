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
#include "propertiesview.h"
#include "propertyeditorwidgets.h"
#include "tilesetchanges.h"
#include "tilesetdocument.h"
#include "tilesetparametersedit.h"
#include "transformmapobjects.h"
#include "utils.h"
#include "variantmapproperty.h"
#include "wangoverlay.h"

#include <QAction>
#include <QCheckBox>
#include <QCoreApplication>
#include <QEvent>
#include <QFileInfo>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QPushButton>
#include <QScopedValueRollback>
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
        QCoreApplication::translate("PreferencesDialog", "CSV"),
        QCoreApplication::translate("PreferencesDialog", "XML (deprecated)"),
        QCoreApplication::translate("PreferencesDialog", "Base64 (uncompressed)"),
        QCoreApplication::translate("PreferencesDialog", "Base64 (gzip compressed)"),
        QCoreApplication::translate("PreferencesDialog", "Base64 (zlib compressed)"),
    };
    QList<int> values {
        Map::CSV,
        Map::XML,
        Map::Base64,
        Map::Base64Gzip,
        Map::Base64Zlib,
    };

    if (compressionSupported(Zstandard)) {
        names.append(QCoreApplication::translate("PreferencesDialog", "Base64 (Zstandard compressed)"));
        values.append(Map::Base64Zstandard);
    }

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

template<> EnumData enumData<BlendMode>()
{
    const QStringList names = {
        QCoreApplication::translate("BlendMode", "Normal"),
        QCoreApplication::translate("BlendMode", "Add"),
        QCoreApplication::translate("BlendMode", "Multiply"),
        QCoreApplication::translate("BlendMode", "Screen"),
        QCoreApplication::translate("BlendMode", "Overlay"),
        QCoreApplication::translate("BlendMode", "Darken"),
        QCoreApplication::translate("BlendMode", "Lighten"),
        QCoreApplication::translate("BlendMode", "Color Dodge"),
        QCoreApplication::translate("BlendMode", "Color Burn"),
        QCoreApplication::translate("BlendMode", "Hard Light"),
        QCoreApplication::translate("BlendMode", "Soft Light"),
        QCoreApplication::translate("BlendMode", "Difference"),
        QCoreApplication::translate("BlendMode", "Exclusion"),
    };
    static const QList<int> values = {
        static_cast<int>(BlendMode::Normal),
        static_cast<int>(BlendMode::Add),
        static_cast<int>(BlendMode::Multiply),
        static_cast<int>(BlendMode::Screen),
        static_cast<int>(BlendMode::Overlay),
        static_cast<int>(BlendMode::Darken),
        static_cast<int>(BlendMode::Lighten),
        static_cast<int>(BlendMode::ColorDodge),
        static_cast<int>(BlendMode::ColorBurn),
        static_cast<int>(BlendMode::HardLight),
        static_cast<int>(BlendMode::SoftLight),
        static_cast<int>(BlendMode::Difference),
        static_cast<int>(BlendMode::Exclusion),
    };
    return { names, values };
}


class FlippingProperty : public IntProperty
{
    Q_OBJECT

public:
    using IntProperty::IntProperty;

    QWidget *createEditor(QWidget *parent) override
    {
        QIcon flipHorizontalIcon(QLatin1String(":images/24/flip-horizontal.png"));
        QIcon flipVerticalIcon(QLatin1String(":images/24/flip-vertical.png"));

        flipHorizontalIcon.addFile(QLatin1String(":images/32/flip-horizontal.png"));
        flipVerticalIcon.addFile(QLatin1String(":images/32/flip-vertical.png"));

        auto editor = new QWidget(parent);

        auto flipHorizontally = new QToolButton(editor);
        flipHorizontally->setToolTip(tr("Flip Horizontally"));
        flipHorizontally->setIcon(flipHorizontalIcon);
        flipHorizontally->setCheckable(true);

        auto flipVertically = new QToolButton(editor);
        flipVertically->setToolTip(tr("Flip Vertically"));
        flipVertically->setIcon(flipVerticalIcon);
        flipVertically->setCheckable(true);

        auto horizontalLayout = new QHBoxLayout(editor);
        horizontalLayout->setContentsMargins(QMargins());
        horizontalLayout->addWidget(flipHorizontally);
        horizontalLayout->addWidget(flipVertically);
        horizontalLayout->addStretch();

        auto syncEditor = [=] {
            const QSignalBlocker horizontalBlocker(flipHorizontally);
            const QSignalBlocker verticalBlocker(flipVertically);
            const auto v = value();
            flipHorizontally->setChecked(v & Cell::FlippedHorizontally);
            flipVertically->setChecked(v & Cell::FlippedVertically);
        };
        auto syncProperty = [=] {
            int flags = 0;
            if (flipHorizontally->isChecked())
                flags |= Cell::FlippedHorizontally;
            if (flipVertically->isChecked())
                flags |= Cell::FlippedVertically;
            setValue(flags);
        };

        syncEditor();

        connect(this, &Property::valueChanged, editor, syncEditor);
        connect(flipHorizontally, &QAbstractButton::toggled, this, syncProperty);
        connect(flipVertically, &QAbstractButton::toggled, this, syncProperty);
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


class TilesetImageProperty : public GroupProperty
{
    Q_OBJECT

public:
    TilesetImageProperty(TilesetDocument *tilesetDocument, QObject *parent)
        : GroupProperty(tr("Tileset Image"), parent)
        , mTilesetDocument(tilesetDocument)
    {}

    DisplayMode displayMode() const override { return DisplayMode::Default; }

    QWidget *createEditor(QWidget *parent) override
    {
        auto editor = new TilesetParametersEdit(parent);
        editor->setTilesetDocument(mTilesetDocument);
        return editor;
    }

private:
    TilesetDocument *mTilesetDocument;
};

static bool propertyValueAffected(Object *currentObject,
                                  Object *changedObject,
                                  const QString &propertyName)
{
    if (currentObject == changedObject)
        return true;

    // Changed property may be inherited
    if (currentObject && currentObject->typeId() == Object::MapObjectType && changedObject->typeId() == Object::TileType) {
        auto tile = static_cast<MapObject*>(currentObject)->cell().tile();
        if (tile == changedObject && !currentObject->hasProperty(propertyName))
            return true;
    }

    return false;
}

static bool objectPropertiesRelevant(Document *document, Object *object)
{
    auto currentObject = document->currentObject();
    if (!currentObject)
        return false;

    if (currentObject == object)
        return true;

    if (currentObject->typeId() == Object::MapObjectType)
        if (static_cast<MapObject*>(currentObject)->cell().tile() == object)
            return true;

    return false;
}

class CustomProperties : public VariantMapProperty
{
    Q_OBJECT

public:
    CustomProperties(QObject *parent = nullptr)
        : VariantMapProperty(tr("Custom Properties"), parent)
    {
        connect(this, &VariantMapProperty::memberValueChanged,
                this, &CustomProperties::setPropertyValue);
    }

    void setDocument(Document *document);

protected:
    void propertyTypesChanged() override
    {
        QScopedValueRollback<bool> propertyTypesChanged(mPropertyTypesChanged, true);
        refresh();
    }

private:
    void onChanged(const ChangeEvent &change) {
        if (change.type != ChangeEvent::ObjectsChanged)
            return;

        auto object = mDocument->currentObject();
        if (!object)
            return;

        auto &objectsChange = static_cast<const ObjectsChangeEvent&>(change);

        if (objectsChange.properties & ObjectsChangeEvent::ClassProperty) {
            if (objectsChange.objects.contains(object)) {
                refresh();
            } else if (object->typeId() == Object::MapObjectType) {
                auto mapObject = static_cast<MapObject*>(object);
                if (auto tile = mapObject->cell().tile()) {
                    if (mapObject->className().isEmpty() && objectsChange.objects.contains(tile))
                        refresh();
                }
            }
        }
    }

    void propertyAdded(Object *object, const QString &) {
        if (mUpdating)
            return;
        if (!objectPropertiesRelevant(mDocument, object))
            return;
        refresh();
    }

    void propertyRemoved(Object *object, const QString &) {
        if (mUpdating)
            return;
        if (!objectPropertiesRelevant(mDocument, object))
            return;
        refresh();
    }

    void propertyChanged(Object *object, const QString &name) {
        if (mUpdating)
            return;
        if (!propertyValueAffected(mDocument->currentObject(), object, name))
            return;
        refresh();
    }

    void propertiesChanged(Object *object) {
        if (!objectPropertiesRelevant(mDocument, object))
            return;
        refresh();
    }

    void refresh();

    void setPropertyValue(const QStringList &path, const QVariant &value);

    bool mUpdating = false;
};


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

class ClassNameProperty : public StringProperty
{
    Q_OBJECT

public:
    ClassNameProperty(Document *document, Object *object, QObject *parent = nullptr)
        : StringProperty(tr("Class"),
                         [this] { return mObject->className(); },
                         [this] (const QString &value) {
                             QUndoStack *undoStack = mDocument->undoStack();
                             undoStack->push(new ChangeClassName(mDocument,
                                                                 mDocument->currentObjects(),
                                                                 value));
                         },
                         parent)
        , mDocument(document)
        , mObject(object)
    {
        updatePlaceholderText();

        connect(mDocument, &Document::changed,
                this, &ClassNameProperty::onChanged);
    }

    QWidget *createEditor(QWidget *parent) override
    {
        auto editor = new ComboBox(parent);
        editor->setEditable(true);
        editor->lineEdit()->setPlaceholderText(placeholderText());
        editor->addItems(classNamesFor(*mObject));
        auto syncEditor = [this, editor] {
            const QSignalBlocker blocker(editor);

            // Avoid affecting cursor position when the text is the same
            const auto v = value();
            if (editor->currentText() != v)
                editor->setCurrentText(v);
        };
        syncEditor();
        connect(this, &Property::valueChanged, editor, syncEditor);
        connect(this, &StringProperty::placeholderTextChanged,
                editor->lineEdit(), &QLineEdit::setPlaceholderText);
        connect(editor, &QComboBox::currentTextChanged, this, &StringProperty::setValue);
        connect(Preferences::instance(), &Preferences::propertyTypesChanged,
                editor, [=] {
            const QSignalBlocker blocker(editor);
            editor->clear();
            editor->addItems(classNamesFor(*mObject));
            syncEditor();
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

        if (objectsEvent.properties & ObjectsChangeEvent::ClassProperty) {
            updatePlaceholderText();
            emit valueChanged();
        }
    }

    void updatePlaceholderText()
    {
        if (mObject->typeId() == Object::MapObjectType && mObject->className().isEmpty())
            setPlaceholderText(static_cast<MapObject*>(mObject)->effectiveClassName());
        else
            setPlaceholderText(QString());
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
        connect(mMapDocument, &MapDocument::mapResized,
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

class ObjectProperties : public GroupProperty
{
    Q_OBJECT

public:
    ObjectProperties(Document *document, Object *object, QObject *parent = nullptr)
        : GroupProperty(parent)
        , mDocument(document)
        , mObject(object)
    {
        mClassProperty = new ClassNameProperty(document, object, this);
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
                        push(new ChangeMapOrientation(mapDocument(), value));
                    });

        mSizeProperty = new MapSizeProperty(mapDocument(), this);

        mTileSizeProperty = new SizeProperty(
                    tr("Tile Size"),
                    [this] {
                        return mapDocument()->map()->tileSize();
                    },
                    [this](const QSize &newSize) {
                        push(new ChangeMapTileSize(mapDocument(), newSize));
                    },
                    this);
        mTileSizeProperty->setMinimum(1);
        mTileSizeProperty->setSuffix(tr(" px"));

        mInfiniteProperty = new BoolProperty(
                    tr("Infinite"),
                    [this] {
                        return map()->infinite();
                    },
                    [this](const bool &value) {
                        push(new ChangeMapInfinite(mapDocument(), value));
                    });
        mInfiniteProperty->setNameOnCheckBox(true);

        mHexSideLengthProperty = new IntProperty(
                    tr("Hex Side Length"),
                    [this] {
                        return map()->hexSideLength();
                    },
                    [this](const int &value) {
                        push(new ChangeMapHexSideLength(mapDocument(), value));
                    });
        mHexSideLengthProperty->setSuffix(tr(" px"));

        mStaggerAxisProperty = new EnumProperty<Map::StaggerAxis>(
                    tr("Stagger Axis"),
                    [this] {
                        return map()->staggerAxis();
                    },
                    [this](Map::StaggerAxis value) {
                        push(new ChangeMapStaggerAxis(mapDocument(), value));
                    });

        mStaggerIndexProperty = new EnumProperty<Map::StaggerIndex>(
                    tr("Stagger Index"),
                    [this] {
                        return map()->staggerIndex();
                    },
                    [this](Map::StaggerIndex value) {
                        push(new ChangeMapStaggerIndex(mapDocument(), value));
                    });

        mParallaxOriginProperty = new PointFProperty(
                    tr("Parallax Origin"),
                    [this] {
                        return map()->parallaxOrigin();
                    },
                    [this](const QPointF &value) {
                        push(new ChangeMapParallaxOrigin(mapDocument(), value));
                    });

        mLayerDataFormatProperty = new EnumProperty<Map::LayerDataFormat>(
                    tr("Layer Data Format"),
                    [this] {
                        return map()->layerDataFormat();
                    },
                    [this](Map::LayerDataFormat value) {
                        push(new ChangeMapLayerDataFormat(mapDocument(), value));
                    });

        mCompressionLevelProperty = new IntProperty(
                    tr("Compression Level"),
                    [this] {
                        return map()->compressionLevel();
                    },
                    [this](const int &value) {
                        push(new ChangeMapCompressionLevel(mapDocument(), value));
                    });

        mChunkSizeProperty = new SizeProperty(
                    tr("Output Chunk Size"),
                    [this] {
                        return map()->chunkSize();
                    },
                    [this](const QSize &value) {
                        push(new ChangeMapChunkSize(mapDocument(), value));
                    });
        mChunkSizeProperty->setMinimum(CHUNK_SIZE_MIN);

        mRenderOrderProperty = new EnumProperty<Map::RenderOrder>(
                    tr("Tile Render Order"),
                    [this] {
                        return map()->renderOrder();
                    },
                    [this](Map::RenderOrder value) {
                        push(new ChangeMapRenderOrder(mapDocument(), value));
                    });

        mBackgroundColorProperty = new ColorProperty(
                    tr("Background Color"),
                    [this] {
                        return map()->backgroundColor();
                    },
                    [this](const QColor &value) {
                        push(new ChangeMapBackgroundColor(mapDocument(), value));
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
        mMapProperties->addProperty(mCompressionLevelProperty);
        mMapProperties->addProperty(mChunkSizeProperty);
        mMapProperties->addSeparator();
        mMapProperties->addProperty(mRenderOrderProperty);
        mMapProperties->addProperty(mBackgroundColorProperty);

        addProperty(mMapProperties);

        updateEnabledState();
        connect(document, &Document::changed,
                this, &MapProperties::onChanged);
    }

private:
    void onChanged(const ChangeEvent &event)
    {
        if (event.type != ChangeEvent::MapChanged)
            return;

        const auto property = static_cast<const MapChangeEvent&>(event).property;
        switch (property) {
        case Map::TileSizeProperty:
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
    BoolProperty *mInfiniteProperty;
    IntProperty *mHexSideLengthProperty;
    Property *mStaggerAxisProperty;
    Property *mStaggerIndexProperty;
    Property *mParallaxOriginProperty;
    Property *mLayerDataFormatProperty;
    Property *mCompressionLevelProperty;
    SizeProperty *mChunkSizeProperty;
    Property *mRenderOrderProperty;
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

        mNameProperty = new StringProperty(
                    tr("Name"),
                    [this] { return layer()->name(); },
                    [this](const QString &value) {
                        push(new SetLayerName(mapDocument(),
                                              mapDocument()->selectedLayers(),
                                              value));
                    });

        mVisibleProperty = new BoolProperty(
                    tr("Visible"),
                    [this] { return layer()->isVisible(); },
                    [this](const bool &value) {
                        push(new SetLayerVisible(mapDocument(),
                                                 mapDocument()->selectedLayers(),
                                                 value));
                    });
        mVisibleProperty->setNameOnCheckBox(true);

        mLockedProperty = new BoolProperty(
                    tr("Locked"),
                    [this] { return layer()->isLocked(); },
                    [this](const bool &value) {
                        push(new SetLayerLocked(mapDocument(),
                                                mapDocument()->selectedLayers(),
                                                value));
                    });
        mLockedProperty->setNameOnCheckBox(true);

        mOpacityProperty = new IntProperty(
                    tr("Opacity"),
                    [this] { return qRound(layer()->opacity() * 100); },
                    [this](const int &value) {
                        push(new SetLayerOpacity(mapDocument(),
                                                 mapDocument()->selectedLayers(),
                                                 qreal(value) / 100));
                    });
        mOpacityProperty->setRange(0, 100);
        mOpacityProperty->setSuffix(tr("%"));
        mOpacityProperty->setSliderEnabled(true);

        mTintColorProperty = new ColorProperty(
                    tr("Tint Color"),
                    [this] { return layer()->tintColor(); },
                    [this](const QColor &value) {
                        push(new SetLayerTintColor(mapDocument(),
                                                   mapDocument()->selectedLayers(),
                                                   value));
                    });

        mBlendModeProperty = new EnumProperty<BlendMode>(
                    tr("Blend Mode"),
                    [this] { return layer()->blendMode(); },
                    [this](BlendMode mode) {
                        push(new SetLayerBlendMode(mapDocument(),
                                                   mapDocument()->selectedLayers(),
                                                   mode));
                    });

        mOffsetProperty = new PointFProperty(
                    tr("Offset"),
                    [this] { return layer()->offset(); },
                    [this](const QPointF &value) {
                        const auto oldValue = layer()->offset();
                        const bool changedX = oldValue.x() != value.x();
                        const bool changedY = oldValue.y() != value.y();

                        QVector<QPointF> offsets;
                        for (const Layer *layer : mapDocument()->selectedLayers())
                            offsets.append(layer->offset());

                        if (changedX) {
                            for (QPointF &offset : offsets)
                                offset.setX(value.x());
                        } else if (changedY) {
                            for (QPointF &offset : offsets)
                                offset.setY(value.y());
                        }

                        push(new SetLayerOffset(mapDocument(),
                                                mapDocument()->selectedLayers(),
                                                offsets));
                    });

        mParallaxFactorProperty = new PointFProperty(
                    tr("Parallax Factor"),
                    [this] { return layer()->parallaxFactor(); },
                    [this](const QPointF &value) {
                        push(new SetLayerParallaxFactor(mapDocument(),
                                                        mapDocument()->selectedLayers(),
                                                        value));
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
        mLayerProperties->addProperty(mBlendModeProperty);
        mLayerProperties->addProperty(mOffsetProperty);
        mLayerProperties->addProperty(mParallaxFactorProperty);

        addProperty(mLayerProperties);

        connect(document, &Document::changed,
                this, &LayerProperties::onChanged);
    }

protected:
    virtual void onChanged(const ChangeEvent &event)
    {
        if (event.type != ChangeEvent::LayerChanged)
            return;

        const auto &layerChange = static_cast<const LayerChangeEvent&>(event);
        if (layerChange.layer != layer())
            return;

        if (layerChange.properties & LayerChangeEvent::NameProperty)
            emit mNameProperty->valueChanged();
        if (layerChange.properties & LayerChangeEvent::VisibleProperty)
            emit mVisibleProperty->valueChanged();
        if (layerChange.properties & LayerChangeEvent::LockedProperty)
            emit mLockedProperty->valueChanged();
        if (layerChange.properties & LayerChangeEvent::OpacityProperty)
            emit mOpacityProperty->valueChanged();
        if (layerChange.properties & LayerChangeEvent::TintColorProperty)
            emit mTintColorProperty->valueChanged();
        if (layerChange.properties & LayerChangeEvent::BlendModeProperty)
            emit mBlendModeProperty->valueChanged();
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

    template <class T>
    QList<T*> selectedLayersOfType(Layer::TypeFlag typeFlag)
    {
        if (!mDocument)
            return {};

        QList<T*> result;
        for (Layer *layer : mapDocument()->selectedLayers())
            if (layer->layerType() == typeFlag)
                result.append(static_cast<T*>(layer));

        return result;
    }

    GroupProperty *mLayerProperties;
    Property *mIdProperty;
    Property *mNameProperty;
    BoolProperty *mVisibleProperty;
    BoolProperty *mLockedProperty;
    IntProperty *mOpacityProperty;
    Property *mTintColorProperty;
    BaseEnumProperty *mBlendModeProperty;
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
                        const auto imageLayers = selectedLayersOfType<ImageLayer>(Layer::ImageLayerType);
                        push(new ChangeImageLayerImageSource(mapDocument(), imageLayers, value));
                    });
        mImageProperty->setFilter(Utils::readableImageFormatsFilter());

        mTransparentColorProperty = new ColorProperty(
                    tr("Transparent Color"),
                    [this] { return imageLayer()->transparentColor(); },
                    [this](const QColor &value) {
                        const auto imageLayers = selectedLayersOfType<ImageLayer>(Layer::ImageLayerType);
                        push(new ChangeImageLayerTransparentColor(mapDocument(), imageLayers, value));
                    });
        mTransparentColorProperty->setAlpha(false);

        mRepeatProperty = new ImageLayerRepeatProperty(
                    tr("Repeat"),
                    [this] { return imageLayer()->repetition(); },
                    [this](const ImageLayer::RepetitionFlags &value) {
                        const bool repeatX = value & ImageLayer::RepeatX;
                        const bool repeatY = value & ImageLayer::RepeatY;
                        const auto imageLayers = selectedLayersOfType<ImageLayer>(Layer::ImageLayerType);
                        if (repeatX != imageLayer()->repeatX())
                            push(new ChangeImageLayerRepeatX(mapDocument(), imageLayers, repeatX));
                        if (repeatY != imageLayer()->repeatY())
                            push(new ChangeImageLayerRepeatY(mapDocument(), imageLayers, repeatY));
                    });

        mImageLayerProperties = new GroupProperty(tr("Image Layer"));
        mImageLayerProperties->addProperty(mImageProperty);
        mImageLayerProperties->addProperty(mTransparentColorProperty);
        mImageLayerProperties->addSeparator();
        mImageLayerProperties->addProperty(mRepeatProperty);

        addProperty(mImageLayerProperties);
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
        if (layerChange.properties & ImageLayerChangeEvent::RepeatProperty)
            emit mRepeatProperty->valueChanged();
    }

    ImageLayer *imageLayer() const
    {
        return static_cast<ImageLayer*>(mObject);
    }

    GroupProperty *mImageLayerProperties;
    UrlProperty *mImageProperty;
    ColorProperty *mTransparentColorProperty;
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
                        const auto objectGroups = selectedLayersOfType<ObjectGroup>(Layer::ObjectGroupType);
                        push(new ChangeObjectGroupColor(mapDocument(), objectGroups, value));
                    });

        mDrawOrderProperty = new EnumProperty<ObjectGroup::DrawOrder>(
                    tr("Draw Order"),
                    [this] { return objectGroup()->drawOrder(); },
                    [this](ObjectGroup::DrawOrder value) {
                        const auto objectGroups = selectedLayersOfType<ObjectGroup>(Layer::ObjectGroupType);
                        push(new ChangeObjectGroupDrawOrder(mapDocument(), objectGroups, value));
                    });

        mObjectGroupProperties = new GroupProperty(tr("Object Layer"));
        mObjectGroupProperties->addProperty(mColorProperty);
        mObjectGroupProperties->addProperty(mDrawOrderProperty);

        addProperty(mObjectGroupProperties);
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
        mTileOffsetProperty->setSuffix(tr(" px"));

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
        mGridSizeProperty->setSuffix(tr(" px"));

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

        // todo: image file name doesn't update in the TilesetParametersEdit
        mTilesetImageProperty = new TilesetImageProperty(document, this);

        mImageProperty = new UrlProperty(
                    tr("Image"),
                    [this] { return tileset()->imageSource(); });

        mTransparentColorProperty = new ColorProperty(
                    tr("Transparent Color"),
                    [this] { return tileset()->transparentColor(); });

        mTileSizeProperty = new SizeProperty(
                    tr("Tile Size"),
                    [this] { return tileset()->tileSize(); });

        mMarginProperty = new IntProperty(
                    tr("Margin"),
                    [this] { return tileset()->margin(); });

        mTileSpacingProperty = new IntProperty(
                    tr("Spacing"),
                    [this] { return tileset()->tileSpacing(); });

        mTileSizeProperty->setSuffix(tr(" px"));
        mMarginProperty->setSuffix(tr(" px"));
        mTileSpacingProperty->setSuffix(tr(" px"));

        mImageProperty->setEnabled(false);
        mTransparentColorProperty->setEnabled(false);
        mTileSizeProperty->setEnabled(false);
        mMarginProperty->setEnabled(false);
        mTileSpacingProperty->setEnabled(false);

        mTilesetImageProperty->addProperty(mImageProperty);
        mTilesetImageProperty->addProperty(mTransparentColorProperty);
        mTilesetImageProperty->addProperty(mTileSizeProperty);
        mTilesetImageProperty->addProperty(mMarginProperty);
        mTilesetImageProperty->addProperty(mTileSpacingProperty);

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

        if (!tileset()->isCollection())
            mTilesetProperties->addProperty(mTilesetImageProperty);

        addProperty(mTilesetProperties);

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
        emit mTransparentColorProperty->valueChanged();
        emit mTileSizeProperty->valueChanged();
        emit mMarginProperty->valueChanged();
        emit mTileSpacingProperty->valueChanged();
    }

    void updateEnabledState()
    {
        const bool collection = tileset()->isCollection();
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
    PointProperty *mTileOffsetProperty;
    Property *mTileRenderSizeProperty;
    Property *mFillModeProperty;
    Property *mBackgroundColorProperty;
    Property *mOrientationProperty;
    SizeProperty *mGridSizeProperty;
    IntProperty *mColumnCountProperty;
    Property *mAllowedTransformationsProperty;
    GroupProperty *mTilesetImageProperty;
    Property *mImageProperty;
    Property *mTransparentColorProperty;
    SizeProperty *mTileSizeProperty;
    IntProperty *mMarginProperty;
    IntProperty *mTileSpacingProperty;
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
        mVisibleProperty->setNameOnCheckBox(true);

        mPositionProperty = new PointFProperty(
                    tr("Position"),
                    [this] {
                        return mapObject()->position();
                    },
                    [this](const QPointF &value) {
                        const auto oldValue = mapObject()->position();
                        const bool changedX = oldValue.x() != value.x();
                        const bool changedY = oldValue.y() != value.y();

                        auto objects = mapDocument()->selectedObjects();
                        QVector<TransformState> states;
                        states.reserve(objects.size());

                        for (MapObject *object : objects) {
                            states.append(TransformState(object));
                            auto &state = states.last();

                            auto position = state.position();

                            if (changedX)
                                position.setX(value.x());
                            if (changedY)
                                position.setY(value.y());

                            state.setPosition(position);
                        }

                        push(new TransformMapObjects(mDocument, objects, states));
                    });

        mBoundsProperty = new RectFProperty(
                    tr("Geometry"),
                    [this] {
                        return mapObject()->bounds();
                    },
                    [this](const QRectF &value) {
                        const auto oldValue = mapObject()->bounds();
                        const bool changedX = oldValue.x() != value.x();
                        const bool changedY = oldValue.y() != value.y();
                        const bool changedWidth = oldValue.width() != value.width();
                        const bool changedHeight = oldValue.height() != value.height();

                        auto objects = mapDocument()->selectedObjects();
                        QVector<TransformState> states;
                        states.reserve(objects.size());

                        for (MapObject *object : objects) {
                            states.append(TransformState(object));
                            auto &state = states.last();

                            auto position = state.position();
                            auto size = state.size();

                            if (changedX)
                                position.setX(value.x());
                            if (changedY)
                                position.setY(value.y());
                            if (changedWidth && object->hasDimensions())
                                size.setWidth(value.width());
                            if (changedHeight && object->hasDimensions())
                                size.setHeight(value.height());

                            state.setPosition(position);
                            state.setSize(size);
                        }

                        push(new TransformMapObjects(mDocument, objects, states));
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

        mFlippingProperty = new FlippingProperty(
                    tr("Flipping"),
                    [this] {
                        return mapObject()->cell().flags();
                    },
                    [this](const int &value) {
                        const int oldValue = mapObject()->cell().flags();
                        const bool changedHorizontally = (oldValue & 1) != (value & 1);
                        const bool changedVertically = (oldValue & 2) != (value & 2);

                        QVector<MapObjectCell> objectChanges;

                        for (MapObject *object : mapDocument()->selectedObjects()) {
                            MapObjectCell mapObjectCell;
                            mapObjectCell.object = object;
                            mapObjectCell.cell = object->cell();
                            if (changedHorizontally)
                                mapObjectCell.cell.setFlippedHorizontally(value & 1);
                            if (changedVertically)
                                mapObjectCell.cell.setFlippedVertically(value & 2);
                            objectChanges.append(mapObjectCell);
                        }

                        auto command = new ChangeMapObjectCells(mDocument, objectChanges);

                        command->setText(QCoreApplication::translate("Undo Commands",
                                                                     "Flip %n Object(s)",
                                                                     nullptr,
                                                                     objectChanges.size()));
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

        if (mapObject()->hasDimensions())
            mObjectProperties->addProperty(mBoundsProperty);
        else
            mObjectProperties->addProperty(mPositionProperty);

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

        addProperty(mObjectProperties);

        connect(document, &Document::changed,
                this, &MapObjectProperties::onChanged);

        updateEnabledState();
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
        if (change.properties & MapObject::PositionProperty) {
            emit mPositionProperty->valueChanged();
            emit mBoundsProperty->valueChanged();
        }
        if (change.properties & MapObject::SizeProperty)
            emit mBoundsProperty->valueChanged();
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
        mBoundsProperty->setEnabled(mapObject()->hasDimensions());
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
        QUndoCommand *command = new ChangeMapObject(mapDocument(), mapObject(),
                                                    property, value);

        if (mapDocument()->selectedObjects().size() == 1) {
            push(command);
            return;
        }

        auto undoStack = mDocument->undoStack();
        undoStack->beginMacro(command->text());
        undoStack->push(command);

        for (MapObject *obj : mapDocument()->selectedObjects()) {
            if (obj != mapObject()) {
                QUndoCommand *cmd = new ChangeMapObject(mapDocument(), obj,
                                                        property, value);
                undoStack->push(cmd);
            }
        }

        mDocument->undoStack()->endMacro();
    }

    GroupProperty *mObjectProperties;
    Property *mIdProperty;
    Property *mTemplateProperty;
    Property *mNameProperty;
    BoolProperty *mVisibleProperty;
    Property *mPositionProperty;
    Property *mBoundsProperty;
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

        addProperty(mTileProperties);

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

        addProperty(mWangSetProperties);

        connect(document, &Document::changed,
                this, &WangSetProperties::onChanged);

        updateEnabledState();
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

        addProperty(mWangColorProperties);

        connect(document, &Document::changed,
                this, &WangColorProperties::onChanged);

        updateEnabledState();
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


PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QWidget{parent}
    , mResetIcon(QIcon(QStringLiteral(":/images/16/edit-clear.png")))
    , mRemoveIcon(QIcon(QStringLiteral(":/images/16/remove.png")))
    , mAddIcon(QIcon(QStringLiteral(":/images/16/add.png")))
    , mRenameIcon(QIcon(QLatin1String(":/images/16/rename.png")))
    , mRootProperty(new GroupProperty())
    , mCustomProperties(new CustomProperties(mRootProperty))
    , mPropertiesView(new PropertiesView(this))
{
    mResetIcon.addFile(QStringLiteral(":/images/24/edit-clear.png"));
    mRemoveIcon.addFile(QStringLiteral(":/images/22/remove.png"));
    mAddIcon.addFile(QStringLiteral(":/images/22/add.png"));

    mRootProperty->addProperty(mCustomProperties);

    mActionAddProperty = new QAction(this);
    mActionAddProperty->setEnabled(false);
    mActionAddProperty->setIcon(mAddIcon);
    connect(mActionAddProperty, &QAction::triggered,
            this, &PropertiesWidget::showAddValueProperty);

    mActionRemoveProperty = new QAction(this);
    mActionRemoveProperty->setEnabled(false);
    mActionRemoveProperty->setIcon(mRemoveIcon);
    mActionRemoveProperty->setShortcuts(QKeySequence::Delete);
    mActionRemoveProperty->setPriority(QAction::LowPriority);
    connect(mActionRemoveProperty, &QAction::triggered,
            this, &PropertiesWidget::removeProperties);

    mActionRenameProperty = new QAction(this);
    mActionRenameProperty->setEnabled(false);
    mActionRenameProperty->setIcon(mRenameIcon);
    mActionRenameProperty->setPriority(QAction::LowPriority);
    connect(mActionRenameProperty, &QAction::triggered,
            this, &PropertiesWidget::renameSelectedProperty);

    Utils::setThemeIcon(mActionAddProperty, "add");
    Utils::setThemeIcon(mActionRemoveProperty, "remove");
    Utils::setThemeIcon(mActionRenameProperty, "rename");

    QToolBar *toolBar = new QToolBar;
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBar->setIconSize(Utils::smallIconSize());
    toolBar->addAction(mActionAddProperty);
    toolBar->addAction(mActionRemoveProperty);
    toolBar->addAction(mActionRenameProperty);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(mPropertiesView);
    layout->addWidget(toolBar);
    setLayout(layout);

    mPropertiesView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mPropertiesView, &QWidget::customContextMenuRequested,
            this, &PropertiesWidget::showContextMenu);
    connect(mPropertiesView, &PropertiesView::selectedPropertiesChanged,
            this, &PropertiesWidget::updateActions);

    retranslateUi();
}

PropertiesWidget::~PropertiesWidget()
{
    // Disconnect to avoid crashing due to signals emitted during destruction
    mPropertiesView->disconnect(this);
}

void PropertiesWidget::setDocument(Document *document)
{
    if (mDocument == document)
        return;

    if (mDocument)
        mDocument->disconnect(this);

    mDocument = document;
    mCustomProperties->setDocument(document);

    if (document) {
        connect(document, &Document::currentObjectChanged,
                this, &PropertiesWidget::currentObjectChanged);
        connect(document, &Document::editCurrentObject,
                this, [this] {
            if (mPropertiesObject)
                mPropertiesObject->expandAll();
            emit bringToFront();
        });

        connect(document, &Document::propertyAdded,
                this, &PropertiesWidget::updateActions);
        connect(document, &Document::propertyRemoved,
                this, &PropertiesWidget::updateActions);

        currentObjectChanged(document->currentObject());
    } else {
        currentObjectChanged(nullptr);
    }
}

GroupProperty *PropertiesWidget::customPropertiesGroup() const
{
    return mCustomProperties;
}

void PropertiesWidget::selectCustomProperty(const QString &name)
{
    if (auto property = mCustomProperties->property(name)) {
        mPropertiesView->focusProperty(property);
        mPropertiesView->setSelectedProperties({ property });
    }
}

void PropertiesWidget::currentObjectChanged(Object *object)
{
    if (mPropertiesObject) {
        // Remember the expanded states
        const auto &subProperties = mPropertiesObject->subProperties();
        for (int i = 0; i < subProperties.size(); ++i) {
            if (auto subGroupProperty = qobject_cast<GroupProperty*>(subProperties.at(i)))
                mExpandedStates[i] = subGroupProperty->isExpanded();
        }

        mRootProperty->deleteProperty(mPropertiesObject);
        mPropertiesObject = nullptr;
    }

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

    // Restore the expanded states
    if (mPropertiesObject) {
        const auto &subProperties = mPropertiesObject->subProperties();
        for (int i = 0; i < subProperties.size(); ++i) {
            if (auto subGroupProperty = qobject_cast<GroupProperty*>(subProperties.at(i)))
                subGroupProperty->setExpanded(mExpandedStates.value(i, true));
        }

        mRootProperty->insertProperty(0, mPropertiesObject);
    }

    mPropertiesView->setRootProperty(object ? mRootProperty : nullptr);

    bool editingTileset = mDocument && mDocument->type() == Document::TilesetDocumentType;
    bool isTileset = object && object->isPartOfTileset();
    bool enabled = object && (!isTileset || editingTileset);

    mPropertiesView->setEnabled(object);
    mActionAddProperty->setEnabled(enabled);
}


void CustomProperties::setDocument(Document *document)
{
    if (mDocument == document)
        return;

    if (mDocument)
        mDocument->disconnect(this);

    mDocument = document;

    if (document) {
        connect(document, &Document::changed, this, &CustomProperties::onChanged);

        connect(document, &Document::currentObjectChanged, this, &CustomProperties::refresh);

        connect(document, &Document::propertyAdded, this, &CustomProperties::propertyAdded);
        connect(document, &Document::propertyRemoved, this, &CustomProperties::propertyRemoved);
        connect(document, &Document::propertyChanged, this, &CustomProperties::propertyChanged);
        connect(document, &Document::propertiesChanged, this, &CustomProperties::propertiesChanged);
    }

    refresh();
}

static bool isAutomappingRulesMap(const MapDocument *mapDocument)
{
    if (!mapDocument)
        return false;

    bool hasInputLayer = false;
    bool hasOutputLayer = false;

    for (const Layer *layer : mapDocument->map()->allLayers()) {
        if (layer->name().startsWith(QLatin1String("input"), Qt::CaseInsensitive))
            hasInputLayer |= layer->isTileLayer();
        else if (layer->name().startsWith(QLatin1String("output"), Qt::CaseInsensitive))
            hasOutputLayer = true;
    }

    return hasInputLayer && hasOutputLayer;
}

static void addAutomappingProperties(Properties &properties, const Object *object)
{
    auto addRuleOptions = [&] {
        mergeProperties(properties, QVariantMap {
            { QStringLiteral("Probability"), 1.0 },
            { QStringLiteral("ModX"), 1 },
            { QStringLiteral("ModY"), 1 },
            { QStringLiteral("OffsetX"), 0 },
            { QStringLiteral("OffsetY"), 0 },
            { QStringLiteral("NoOverlappingOutput"), false },
            { QStringLiteral("Disabled"), false },
            { QStringLiteral("IgnoreLock"), false },
        });
    };

    switch (object->typeId()) {
    case Object::LayerType: {
        auto layer = static_cast<const Layer*>(object);

        if (layer->name().startsWith(QLatin1String("input"), Qt::CaseInsensitive)) {
            mergeProperties(properties, QVariantMap {
                { QStringLiteral("AutoEmpty"), false },
                { QStringLiteral("IgnoreHorizontalFlip"), false },
                { QStringLiteral("IgnoreVerticalFlip"), false },
                { QStringLiteral("IgnoreDiagonalFlip"), false },
                // { QStringLiteral("IgnoreHexRotate120"), false },
             });
        } else if (layer->name().startsWith(QLatin1String("output"), Qt::CaseInsensitive)) {
            mergeProperties(properties, QVariantMap {
                { QStringLiteral("Probability"), 1.0 },
            });
        }
        break;
    }
    case Object::MapType:
        mergeProperties(properties, QVariantMap {
            { QStringLiteral("DeleteTiles"), false },
            { QStringLiteral("MatchOutsideMap"), false },
            { QStringLiteral("OverflowBorder"), false },
            { QStringLiteral("WrapBorder"), false },
            { QStringLiteral("AutomappingRadius"), 0 },
            { QStringLiteral("NoOverlappingOutput"), false },
            { QStringLiteral("MatchInOrder"), false },
        });
        addRuleOptions();
        break;
    case Object::MapObjectType: {
        if (auto objectGroup = static_cast<const MapObject*>(object)->objectGroup())
            if (objectGroup->name().compare(QLatin1String("rule_options"), Qt::CaseInsensitive) == 0)
                addRuleOptions();
        break;
    }
    default:
        break;
    }
}

void CustomProperties::refresh()
{
    if (!mDocument || !mDocument->currentObject()) {
        setValue({});
        return;
    }

    auto currentObject = mDocument->currentObject();

    // Suggest properties inherited from the class, tile or template.
    Properties suggestedProperties = currentObject->inheritedProperties();

    if (isAutomappingRulesMap(qobject_cast<const MapDocument*>(mDocument)))
        addAutomappingProperties(suggestedProperties, currentObject);

    setValue(currentObject->properties(),
             suggestedProperties);

    const bool editingTileset = mDocument->type() == Document::TilesetDocumentType;
    const bool partOfTileset = mDocument->currentObject()->isPartOfTileset();
    setEnabled(!partOfTileset || editingTileset);
}

void CustomProperties::setPropertyValue(const QStringList &path, const QVariant &value)
{
    const auto objects = mDocument->currentObjects();
    if (!objects.isEmpty()) {
        QScopedValueRollback<bool> updating(mUpdating, true);
        if (path.size() > 1 || value.isValid())
            mDocument->undoStack()->push(new SetProperty(mDocument, objects, path, value));
        else
            mDocument->undoStack()->push(new RemoveProperty(mDocument, objects, path.first()));
    }
}


void PropertiesWidget::updateActions()
{
    const auto properties = mPropertiesView->selectedProperties();
    bool editingTileset = mDocument && mDocument->type() == Document::TilesetDocumentType;
    bool isTileset = mDocument && mDocument->currentObject() && mDocument->currentObject()->isPartOfTileset();
    bool canModify = mDocument && !properties.isEmpty() && (!isTileset || editingTileset);

    // Disable remove and rename actions when none of the selected objects
    // actually have the selected property (it may be inherited).
    if (canModify) {
        for (auto property : properties) {
            if (!anyObjectHasProperty(mDocument->currentObjects(), property->name())) {
                canModify = false;
                break;
            }
        }
    }

    mActionRemoveProperty->setEnabled(canModify);
    mActionRenameProperty->setEnabled(canModify && properties.size() == 1);
}

void PropertiesWidget::cutProperties()
{
    if (copyProperties())
        removeProperties();
}

bool PropertiesWidget::copyProperties()
{
    Object *object = mDocument ? mDocument->currentObject() : nullptr;
    if (!object)
        return false;

    const auto selectedProperties = mPropertiesView->selectedProperties();
    if (selectedProperties.isEmpty())
        return false;

    Properties properties;
    for (auto property : selectedProperties) {
        const QString name = property->name();
        const QVariant value = object->property(name);
        if (!value.isValid())
            return false;

        properties.insert(name, value);
    }

    ClipboardManager::instance()->setProperties(properties);

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

        // Make sure Custom Properties are expanded
        mCustomProperties->setExpanded(true);
    }

    // Select the pasted properties
    QList<Property*> selectedProperties;
    for (const QString &name : pastedProperties.keys()) {
        if (auto property = mCustomProperties->property(name))
            selectedProperties.append(property);
    }
    if (!selectedProperties.isEmpty()) {
        mPropertiesView->focusProperty(selectedProperties.first(), PropertiesView::FocusRow);
        mPropertiesView->setSelectedProperties(selectedProperties);
    }
}

void PropertiesWidget::showAddValueProperty()
{
    if (!mAddValueProperty) {
        mAddValueProperty = new AddValueProperty(mCustomProperties);

        connect(mAddValueProperty, &Property::addRequested, this, [this] {
            addProperty(mAddValueProperty->name(), mAddValueProperty->value());
            mCustomProperties->deleteProperty(mAddValueProperty);
        });
        connect(mAddValueProperty, &Property::removeRequested, this, [this] {
            mCustomProperties->deleteProperty(mAddValueProperty);
        });

        mCustomProperties->addProperty(mAddValueProperty);
    }

    mPropertiesView->focusProperty(mAddValueProperty, PropertiesView::FocusLabel);
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

    selectCustomProperty(name);
}

void PropertiesWidget::removeProperties()
{
    Object *object = mDocument->currentObject();
    if (!object)
        return;

    const auto properties = mPropertiesView->selectedProperties();

    QStringList propertyNames;
    for (auto property : properties)
        propertyNames.append(property->name());

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
}

void PropertiesWidget::renameSelectedProperty()
{
    const auto properties = mPropertiesView->selectedProperties();
    if (properties.size() != 1)
        return;

    renameProperty(properties.first()->name());
}

void PropertiesWidget::renameProperty(const QString &name)
{
    QInputDialog *dialog = new QInputDialog(mPropertiesView);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setInputMode(QInputDialog::TextInput);
    dialog->setLabelText(QCoreApplication::translate("Tiled::PropertiesDock", "Name:"));
    dialog->setTextValue(name);
    dialog->setWindowTitle(QCoreApplication::translate("Tiled::PropertiesDock", "Rename Property"));

    connect(dialog, &QInputDialog::textValueSelected, this, [=] (const QString &newName) {
        if (newName.isEmpty())
            return;
        if (newName == name)
            return;

        QUndoStack *undoStack = mDocument->undoStack();
        undoStack->push(new RenameProperty(mDocument, mDocument->currentObjects(), name, newName));
    });

    dialog->open();
}

void PropertiesWidget::showContextMenu(const QPoint &pos)
{
    const Object *object = mDocument->currentObject();
    if (!object)
        return;

    const auto properties = mPropertiesView->selectedProperties();
    const bool customPropertiesSelected = !properties.isEmpty();

    bool currentObjectHasAllProperties = true;
    QStringList propertyNames;
    for (auto property : properties) {
        propertyNames.append(property->name());

        if (!object->hasProperty(property->name()))
            currentObjectHasAllProperties = false;
    }

    QMenu contextMenu(mPropertiesView);

    // Add properties specific to the just clicked property
    if (auto focusedProperty = mPropertiesView->focusedProperty()) {
        focusedProperty->addContextMenuActions(&contextMenu);

        // Provide the Add, Remove and Reset actions also here
        if (const auto actions = focusedProperty->actions()) {
            if (!contextMenu.isEmpty())
                contextMenu.addSeparator();

            // Note: No "Remove" added here since it's covered below

            if (actions.testFlag(Property::Action::Add)) {
                QAction *add = contextMenu.addAction(mAddIcon, tr("Add"),
                                                     focusedProperty, &Property::addRequested);
                add->setEnabled(!actions.testFlag(Property::Action::AddDisabled));
                Utils::setThemeIcon(add, "add");
            }

            if (actions.testFlag(Property::Action::Reset)) {
                QAction *reset = contextMenu.addAction(mResetIcon, tr("Reset"),
                                                       focusedProperty, &Property::resetRequested);
                reset->setEnabled(focusedProperty->isModified());
                Utils::setThemeIcon(reset, "edit-clear");
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

    const QPoint globalPos = mPropertiesView->mapToGlobal(pos);
    const QAction *selectedItem = contextMenu.exec(globalPos);

    if (selectedItem && convertMenu && selectedItem->parentWidget() == convertMenu) {
        QUndoStack *undoStack = mDocument->undoStack();
        undoStack->beginMacro(QCoreApplication::translate("Tiled::PropertiesDock", "Convert Property/Properties", nullptr, properties.size()));

        const int toType = selectedItem->data().toInt();

        for (const QString &propertyName : propertyNames) {
            QList<Object*> objects;
            QVariantList values;

            for (auto obj : mDocument->currentObjects()) {
                QVariant propertyValue = obj->property(propertyName);
                if (propertyValue.convert(toType)) {
                    objects.append(obj);
                    values.append(propertyValue);
                }
            }

            undoStack->push(new SetProperty(mDocument, objects,
                                            QStringList { propertyName },
                                            values));
        }

        undoStack->endMacro();

        // Restore selected properties
        QList<Property*> selectedProperties;
        for (const QString &name : propertyNames) {
            if (auto property = mCustomProperties->property(name))
                selectedProperties.append(property);
        }
        if (!selectedProperties.isEmpty())
            mPropertiesView->setSelectedProperties(selectedProperties);
    }
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
