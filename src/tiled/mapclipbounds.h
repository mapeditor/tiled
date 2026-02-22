/*
 * mapclipbounds.h
 *
 * This file is part of Tiled.
 */

#pragma once

#include "map.h"
#include "maprenderer.h"
#include "properties.h"

#include <QRectF>
#include <QRegularExpression>
#include <QStringList>

#include <optional>

namespace Tiled {

namespace Internal {

inline QVariant unwrapPropertyValue(QVariant value)
{
    while (value.userType() == propertyValueId())
        value = value.value<PropertyValue>().value;

    return value;
}

inline bool variantToDouble(const QVariant &value, double &result)
{
    bool ok = false;
    result = unwrapPropertyValue(value).toDouble(&ok);
    return ok;
}

inline std::optional<QRectF> clipRectFromVariant(const QVariant &rawValue)
{
    const QVariant value = unwrapPropertyValue(rawValue);

    switch (value.userType()) {
    case QMetaType::QRect:
        return value.toRect();
    case QMetaType::QRectF:
        return value.toRectF();
    case QMetaType::QVariantList: {
        const QVariantList list = value.toList();
        if (list.size() != 4)
            return std::nullopt;

        double x, y, width, height;
        if (!variantToDouble(list.at(0), x) ||
            !variantToDouble(list.at(1), y) ||
            !variantToDouble(list.at(2), width) ||
            !variantToDouble(list.at(3), height))
            return std::nullopt;

        return QRectF(x, y, width, height);
    }
    case QMetaType::QVariantMap: {
        const QVariantMap map = value.toMap();
        double x, y, width, height;
        if (!variantToDouble(map.value(QStringLiteral("x")), x) ||
            !variantToDouble(map.value(QStringLiteral("y")), y) ||
            !variantToDouble(map.value(QStringLiteral("width")), width) ||
            !variantToDouble(map.value(QStringLiteral("height")), height))
            return std::nullopt;

        return QRectF(x, y, width, height);
    }
    case QMetaType::QString: {
        const QStringList numbers = value.toString().split(QRegularExpression(QStringLiteral("[\\s,;]+")), Qt::SkipEmptyParts);
        if (numbers.size() != 4)
            return std::nullopt;

        bool okX = false;
        bool okY = false;
        bool okW = false;
        bool okH = false;

        const double x = numbers.at(0).toDouble(&okX);
        const double y = numbers.at(1).toDouble(&okY);
        const double width = numbers.at(2).toDouble(&okW);
        const double height = numbers.at(3).toDouble(&okH);

        if (!okX || !okY || !okW || !okH)
            return std::nullopt;

        return QRectF(x, y, width, height);
    }
    default:
        return std::nullopt;
    }
}

inline std::optional<QRectF> customClipRectInTiles(const Map &map)
{
    static const QStringList propertyNames = {
        QStringLiteral("clipRect"),
        QStringLiteral("clipBounds"),
    };

    for (const QString &name : propertyNames) {
        const QVariant value = map.resolvedProperty(name);
        if (!value.isValid())
            continue;

        const auto rect = clipRectFromVariant(value);
        if (rect && rect->isValid() && rect->width() > 0 && rect->height() > 0)
            return rect;
    }

    return std::nullopt;
}

inline QRectF effectiveClipBounds(const MapRenderer &renderer)
{
    if (const auto tileRect = customClipRectInTiles(*renderer.map()))
        return renderer.boundingRect(tileRect->toAlignedRect());

    return renderer.mapBoundingRect();
}

} // namespace Internal

} // namespace Tiled
