/*
 * editablemapobject.h
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include "editableobject.h"
#include "mapobject.h"

#include <QJSValue>

namespace Tiled {

class EditableMap;
class EditableObjectGroup;
class EditableTile;

class Font : public QFont
{
    Q_GADGET

    Q_PROPERTY(QString family READ family WRITE setFamily)
    Q_PROPERTY(int pixelSize READ pixelSize WRITE setPixelSize)
    Q_PROPERTY(bool bold READ bold WRITE setBold)
    Q_PROPERTY(bool italic READ italic WRITE setItalic)
    Q_PROPERTY(bool underline READ underline WRITE setUnderline)
    Q_PROPERTY(bool strikeOut READ strikeOut WRITE setStrikeOut)
    Q_PROPERTY(bool kerning READ kerning WRITE setKerning)

public:
    Font() = default;
    Font(const QFont &font) : QFont(font) {}
};

class EditableMapObject : public EditableObject
{
    Q_OBJECT

    Q_PROPERTY(int id READ id)
    Q_PROPERTY(Shape shape READ shape WRITE setShape)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString type READ type WRITE setType)
    Q_PROPERTY(qreal x READ x WRITE setX)
    Q_PROPERTY(qreal y READ y WRITE setY)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
    Q_PROPERTY(qreal width READ width WRITE setWidth)
    Q_PROPERTY(qreal height READ height WRITE setHeight)
    Q_PROPERTY(QSizeF size READ size WRITE setSize)
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)
    Q_PROPERTY(QJSValue polygon READ polygon WRITE setPolygon)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(Tiled::Font font READ font WRITE setFont)
    Q_PROPERTY(Qt::Alignment textAlignment READ textAlignment WRITE setTextAlignment)
    Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(Tiled::EditableTile *tile READ tile WRITE setTile)
    Q_PROPERTY(bool tileFlippedHorizontally READ tileFlippedHorizontally WRITE setTileFlippedHorizontally)
    Q_PROPERTY(bool tileFlippedVertically READ tileFlippedVertically WRITE setTileFlippedVertically)
//    Q_PROPERTY(const ObjectTemplate *mObjectTemplate)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected)
    Q_PROPERTY(Tiled::EditableObjectGroup *layer READ layer)
    Q_PROPERTY(Tiled::EditableMap *map READ map)
//    Q_PROPERTY(ChangedProperties mChangedProperties)

public:
    // Synchronized with MapObject::Shape
    enum Shape {
        Rectangle,
        Polygon,
        Polyline,
        Ellipse,
        Text,
        Point,
    };
    Q_ENUM(Shape)

    Q_INVOKABLE explicit EditableMapObject(const QString &name = QString(),
                                           QObject *parent = nullptr);

    Q_INVOKABLE explicit EditableMapObject(Shape shape,
                                           const QString &name = QString(),
                                           QObject *parent = nullptr);

    EditableMapObject(EditableAsset *asset,
                      MapObject *mapObject,
                      QObject *parent = nullptr);

    ~EditableMapObject() override;

    int id() const;
    Shape shape() const;
    QString name() const;
    QString type() const;
    qreal x() const;
    qreal y() const;
    QPointF pos() const;
    qreal width() const;
    qreal height() const;
    QSizeF size() const;
    qreal rotation() const;
    bool isVisible() const;
    QJSValue polygon() const;
    QString text() const;
    Font font() const;
    Qt::Alignment textAlignment() const;
    bool wordWrap() const;
    QColor textColor() const;
    EditableTile *tile() const;
    bool tileFlippedHorizontally() const;
    bool tileFlippedVertically() const;
    bool isSelected() const;
    EditableObjectGroup *layer() const;
    EditableMap *map() const;

    MapObject *mapObject() const;

    void detach();
    void attach(EditableMap *map);
    void hold();
    void release();

public slots:
    void setShape(Shape shape);
    void setName(QString name);
    void setType(QString type);
    void setX(qreal x);
    void setY(qreal y);
    void setPos(QPointF pos);
    void setWidth(qreal width);
    void setHeight(qreal height);
    void setSize(QSizeF size);
    void setRotation(qreal rotation);
    void setVisible(bool visible);
    void setPolygon(QJSValue polygon);
    void setText(const QString &text);
    void setFont(const Font &font);
    void setTextAlignment(Qt::Alignment textAlignment);
    void setWordWrap(bool wordWrap);
    void setTextColor(const QColor &textColor);
    void setTile(EditableTile *tile);
    void setTileFlippedHorizontally(bool tileFlippedHorizontally);
    void setTileFlippedVertically(bool tileFlippedVertically);
    void setSelected(bool selected);

private:
    void setMapObjectProperty(MapObject::Property property, const QVariant &value);

    std::unique_ptr<MapObject> mDetachedMapObject;
};


inline int EditableMapObject::id() const
{
    return mapObject()->id();
}

inline EditableMapObject::Shape EditableMapObject::shape() const
{
    return static_cast<Shape>(mapObject()->shape());
}

inline QString EditableMapObject::name() const
{
    return mapObject()->name();
}

inline QString EditableMapObject::type() const
{
    return mapObject()->type();
}

inline qreal EditableMapObject::x() const
{
    return mapObject()->x();
}

inline qreal EditableMapObject::y() const
{
    return mapObject()->y();
}

inline QPointF EditableMapObject::pos() const
{
    return mapObject()->position();
}

inline qreal EditableMapObject::width() const
{
    return mapObject()->width();
}

inline qreal EditableMapObject::height() const
{
    return mapObject()->height();
}

inline QSizeF EditableMapObject::size() const
{
    return mapObject()->size();
}

inline qreal EditableMapObject::rotation() const
{
    return mapObject()->rotation();
}

inline bool EditableMapObject::isVisible() const
{
    return mapObject()->isVisible();
}

inline QString EditableMapObject::text() const
{
    return mapObject()->textData().text;
}

inline Font EditableMapObject::font() const
{
    return mapObject()->textData().font;
}

inline Qt::Alignment EditableMapObject::textAlignment() const
{
    return mapObject()->textData().alignment;
}

inline bool EditableMapObject::wordWrap() const
{
    return mapObject()->textData().wordWrap;
}

inline QColor EditableMapObject::textColor() const
{
    return mapObject()->textData().color;
}

inline bool EditableMapObject::tileFlippedHorizontally() const
{
    return mapObject()->cell().flippedHorizontally();
}

inline bool EditableMapObject::tileFlippedVertically() const
{
    return mapObject()->cell().flippedVertically();
}

inline MapObject *EditableMapObject::mapObject() const
{
    return static_cast<MapObject*>(object());
}

inline void EditableMapObject::setX(qreal x)
{
    setPos(QPointF(x, y()));
}

inline void EditableMapObject::setY(qreal y)
{
    setPos(QPointF(x(), y));
}

inline void EditableMapObject::setWidth(qreal width)
{
    setSize(QSizeF(width, height()));
}

inline void EditableMapObject::setHeight(qreal height)
{
    setSize(QSizeF(width(), height));
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Font)
