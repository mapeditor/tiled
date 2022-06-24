/*
 * variantpropertymanager.cpp
 * Copyright (C) 2006 Trolltech ASA. All rights reserved. (GPLv2)
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

#include "variantpropertymanager.h"

#include "documentmanager.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectmodel.h"
#include "objectgroup.h"
#include "textpropertyedit.h"
#include "tilesetdocument.h"

#include <QFileInfo>

namespace Tiled {

class TilesetParametersPropertyType {};

} // namespace Tiled

// Needs to be up here rather than at the bottom of the file to make a
// static_assert in qMetaTypeId work (as of C++11)
Q_DECLARE_METATYPE(Tiled::TilesetParametersPropertyType)

namespace Tiled {

MapObject *DisplayObjectRef::object() const
{
    if (!mapDocument || ref.id <= 0)
        return nullptr;
    return mapDocument->map()->findObjectById(ref.id);
}


VariantPropertyManager::VariantPropertyManager(QObject *parent)
    : QtVariantPropertyManager(parent)
    , mFilterAttribute(QStringLiteral("filter"))
    , mDirectoryAttribute(QStringLiteral("directory"))
    , mSuggestionsAttribute(QStringLiteral("suggestions"))
    , mMultilineAttribute(QStringLiteral("multiline"))
    , mImageMissingIcon(QStringLiteral("://images/16/image-missing.png"))
{
    mImageMissingIcon.addPixmap(QPixmap(QStringLiteral("://images/32/image-missing.png")));

    connect(this, &QtVariantPropertyManager::valueChanged,
            this, &VariantPropertyManager::slotValueChanged);
    connect(this, &QtAbstractPropertyManager::propertyDestroyed,
            this, &VariantPropertyManager::slotPropertyDestroyed);
}

VariantPropertyManager::~VariantPropertyManager()
{
    // Disconnect early since these functions may be called during destruction
    disconnect(this, &QtVariantPropertyManager::valueChanged,
               this, &VariantPropertyManager::slotValueChanged);
    disconnect(this, &QtAbstractPropertyManager::propertyDestroyed,
               this, &VariantPropertyManager::slotPropertyDestroyed);
}

QVariant VariantPropertyManager::value(const QtProperty *property) const
{
    if (mValues.contains(property))
        return mValues[property];
    if (m_alignValues.contains(property))
        return QVariant::fromValue(m_alignValues.value(property));
    return QtVariantPropertyManager::value(property);
}

bool VariantPropertyManager::isPropertyTypeSupported(int propertyType) const
{
    if (propertyType == filePathTypeId()
            || propertyType == displayObjectRefTypeId()
            || propertyType == tilesetParametersTypeId()
            || propertyType == alignmentTypeId()
            || propertyType == unstyledGroupTypeId())
        return true;
    return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
}

int VariantPropertyManager::valueType(int propertyType) const
{
    if (propertyType == filePathTypeId())
        return propertyType;
    if (propertyType == displayObjectRefTypeId())
        return propertyType;
    if (propertyType == tilesetParametersTypeId())
        return qMetaTypeId<TilesetDocument*>();
    if (propertyType == alignmentTypeId())
        return propertyType;
    if (propertyType == unstyledGroupTypeId())
        return QMetaType::QVariantMap; // allows storing any child values
    return QtVariantPropertyManager::valueType(propertyType);
}

QStringList VariantPropertyManager::attributes(int propertyType) const
{
    if (propertyType == filePathTypeId()) {
        return {
            mFilterAttribute,
            mDirectoryAttribute
        };
    }
    return QtVariantPropertyManager::attributes(propertyType);
}

int VariantPropertyManager::attributeType(int propertyType,
                                          const QString &attribute) const
{
    if (propertyType == filePathTypeId()) {
        if (attribute == mFilterAttribute)
            return QMetaType::QString;
        if (attribute == mDirectoryAttribute)
            return QMetaType::Bool;
        return 0;
    }
    return QtVariantPropertyManager::attributeType(propertyType, attribute);
}

QVariant VariantPropertyManager::attributeValue(const QtProperty *property,
                                                const QString &attribute) const
{
    if (mFilePathAttributes.contains(property)) {
        if (attribute == mFilterAttribute)
            return mFilePathAttributes[property].filter;
        if (attribute == mDirectoryAttribute)
            return mFilePathAttributes[property].directory;
        return QVariant();
    }
    if (mStringAttributes.contains(property)) {
        if (attribute == mSuggestionsAttribute)
            return mStringAttributes[property].suggestions;
        if (attribute == mMultilineAttribute)
            return mStringAttributes[property].multiline;
    }

    return QtVariantPropertyManager::attributeValue(property, attribute);
}

int VariantPropertyManager::tilesetParametersTypeId()
{
    return qMetaTypeId<TilesetParametersPropertyType>();
}

int VariantPropertyManager::alignmentTypeId()
{
    return qMetaTypeId<Qt::Alignment>();
}

int VariantPropertyManager::displayObjectRefTypeId()
{
    return qMetaTypeId<DisplayObjectRef>();
}

int VariantPropertyManager::unstyledGroupTypeId()
{
    return qMetaTypeId<UnstyledGroup>();
}

QString VariantPropertyManager::objectRefLabel(const MapObject &object)
{
    const QString &className = object.effectiveClassName();

    QString label = tr("%1: ").arg(object.id());
    if (!object.name().isEmpty()) {
        label.append(object.name());
        if (!className.isEmpty())
            label.append(tr(" (%1)").arg(className));
    } else if (!className.isEmpty())
        label.append(tr("(%1)").arg(className));
    else
        label.append(tr("Unnamed object"));

    return label;
}

QString VariantPropertyManager::valueText(const QtProperty *property) const
{
    if (mValues.contains(property)) {
        QVariant value = mValues[property];
        int typeId = propertyType(property);

        if (typeId == displayObjectRefTypeId()) {
            const auto ref = value.value<DisplayObjectRef>();

            if (ref.id() == 0)
                return tr("Unset");

            if (auto object = ref.object())
                return objectRefLabel(*object);

            return tr("%1: Object not found").arg(QString::number(ref.id()));
        }

        if (typeId == filePathTypeId()) {
            FilePath filePath = value.value<FilePath>();
            QString fileName = filePath.url.fileName();
            if (fileName.isEmpty()) {
                QString path = filePath.url.toLocalFile();
                if (path.endsWith(QLatin1Char('/')))
                    path.chop(1);
                fileName = QFileInfo(path).fileName();
            }
            return fileName;
        }

        if (typeId == tilesetParametersTypeId()) {
            if (TilesetDocument *tilesetDocument = value.value<TilesetDocument*>())
                return tilesetDocument->tileset()->imageSource().fileName();
        }

        return value.toString();
    }

    if (m_alignValues.contains(const_cast<QtProperty *>(property))) {
        const Qt::Alignment v = m_alignValues.value(const_cast<QtProperty *>(property));
        return tr("%1, %2").arg(indexHToString(alignToIndexH(v)),
                                indexVToString(alignToIndexV(v)));
    }

    auto stringAttributesIt = mStringAttributes.find(property);
    if (stringAttributesIt != mStringAttributes.end()) {
        if ((*stringAttributesIt).multiline)
            return escapeNewlines(value(property).toString());
    }

    return QtVariantPropertyManager::valueText(property);
}

QIcon VariantPropertyManager::valueIcon(const QtProperty *property) const
{
    if (mValues.contains(property)) {
        const QVariant value = mValues[property];
        const int typeId = propertyType(property);

        if (typeId == displayObjectRefTypeId()) {
            const DisplayObjectRef ref = value.value<DisplayObjectRef>();
            if (auto object = ref.object())
                return ObjectIconManager::instance().iconForObject(*object);
        }

        if (typeId == unstyledGroupTypeId())
            return QIcon();

        QString filePath;

        // TODO: Needs a special icon for remote files
        if (typeId == filePathTypeId()) {
            const FilePath fp = value.value<FilePath>();
            filePath = fp.url.toLocalFile();
        }

        if (typeId == tilesetParametersTypeId()) {
            if (TilesetDocument *tilesetDocument = value.value<TilesetDocument*>())
                filePath = tilesetDocument->tileset()->imageSource().toLocalFile();
        }

        // TODO: This assumes the file path is an image reference. It should be
        // replaced with a more generic icon.
        if (filePath.isEmpty() || !QFile::exists(filePath))
            return QIcon::fromTheme(QLatin1String("image-missing"), mImageMissingIcon);
        else
            return mIconProvider.icon(QFileInfo(filePath));
    }

    return QtVariantPropertyManager::valueIcon(property);
}

void VariantPropertyManager::setValue(QtProperty *property, const QVariant &value)
{
    if (mValues.contains(property)) {
        QVariant &storedValue = mValues[property];
        if (storedValue == value)
            return;
        storedValue = value;
        emit propertyChanged(property);
        emit valueChanged(property, value);
        return;
    } else if (m_alignValues.contains(property)) {
        if (value.userType() != qMetaTypeId<Qt::Alignment>() && !value.canConvert(qMetaTypeId<Qt::Alignment>()))
            return;

        const Qt::Alignment v = value.value<Qt::Alignment>();
        const Qt::Alignment val = m_alignValues.value(property);

        if (val == v)
            return;

        QtVariantProperty *alignH = variantProperty(m_propertyToAlignH.value(property));
        QtVariantProperty *alignV = variantProperty(m_propertyToAlignV.value(property));

        if (alignH)
            alignH->setValue(alignToIndexH(v));
        if (alignV)
            alignV->setValue(alignToIndexV(v));

        m_alignValues[property] = v;

        emit QtVariantPropertyManager::valueChanged(property, QVariant::fromValue(v));
        emit propertyChanged(property);

        return;
    }
    QtVariantPropertyManager::setValue(property, value);
}

void VariantPropertyManager::setAttribute(QtProperty *property,
                                          const QString &attribute,
                                          const QVariant &val)
{
    if (mFilePathAttributes.contains(property)) {
        FilePathAttributes &attributes = mFilePathAttributes[property];
        if (attribute == mFilterAttribute) {
            if (val.userType() != QMetaType::QString && !val.canConvert(QMetaType::QString))
                return;
            QString filter = val.toString();
            if (attributes.filter == filter)
                return;
            attributes.filter = filter;
            emit attributeChanged(property, attribute, filter);
        } else if (attribute == mDirectoryAttribute) {
            bool directory = val.toBool();
            if (attributes.directory == directory)
                return;
            attributes.directory = directory;
            emit attributeChanged(property, attribute, directory);
        }
        return;
    }

    if (mStringAttributes.contains(property)) {
        if (attribute == mSuggestionsAttribute) {
            mStringAttributes[property].suggestions = val.toStringList();
            return;
        }
        if (attribute == mMultilineAttribute) {
            mStringAttributes[property].multiline = val.toBool();
            return;
        }
    }

    QtVariantPropertyManager::setAttribute(property, attribute, val);
}

void VariantPropertyManager::initializeProperty(QtProperty *property)
{
    const int type = propertyType(property);
    if (type == filePathTypeId()
            || type == displayObjectRefTypeId()
            || type == tilesetParametersTypeId()
            || type == unstyledGroupTypeId()) { // for storing hash map
        mValues[property] = QVariant();
        if (type == filePathTypeId())
            mFilePathAttributes[property] = FilePathAttributes();
    } else if (type == QMetaType::QString) {
        mStringAttributes[property] = StringAttributes();
    } else if (type == alignmentTypeId()) {
        const Qt::Alignment align = Qt::AlignLeft | Qt::AlignVCenter;
        m_alignValues[property] = align;

        QtVariantProperty *alignH = addProperty(enumTypeId(), tr("Horizontal"));
        QStringList namesH;
        namesH << indexHToString(0) << indexHToString(1) << indexHToString(2) << indexHToString(3);
        alignH->setAttribute(QStringLiteral("enumNames"), namesH);
        alignH->setValue(alignToIndexH(align));
        m_propertyToAlignH[property] = alignH;
        m_alignHToProperty[alignH] = property;
        property->addSubProperty(alignH);

        QtVariantProperty *alignV = addProperty(enumTypeId(), tr("Vertical"));
        QStringList namesV;
        namesV << indexVToString(0) << indexVToString(1) << indexVToString(2);
        alignV->setAttribute(QStringLiteral("enumNames"), namesV);
        alignV->setValue(alignToIndexV(align));
        m_propertyToAlignV[property] = alignV;
        m_alignVToProperty[alignV] = property;
        property->addSubProperty(alignV);
    }


    QtVariantPropertyManager::initializeProperty(property);
}

void VariantPropertyManager::uninitializeProperty(QtProperty *property)
{
    mValues.remove(property);
    mFilePathAttributes.remove(property);
    mStringAttributes.remove(property);
    m_alignValues.remove(property);

    QtProperty *alignH = m_propertyToAlignH.value(property);
    if (alignH) {
        delete alignH;
        m_alignHToProperty.remove(alignH);
    }
    QtProperty *alignV = m_propertyToAlignV.value(property);
    if (alignV) {
        delete alignV;
        m_alignVToProperty.remove(alignV);
    }

    QtVariantPropertyManager::uninitializeProperty(property);
}

void VariantPropertyManager::slotValueChanged(QtProperty *property, const QVariant &value)
{
    if (QtProperty *alignProperty = m_alignHToProperty.value(property)) {
        const Qt::Alignment v = m_alignValues.value(alignProperty);
        const Qt::Alignment newValue = indexHToAlign(value.toInt()) | indexVToAlign(alignToIndexV(v));
        if (v == newValue)
            return;

        variantProperty(alignProperty)->setValue(QVariant::fromValue(newValue));
    } else if (QtProperty *alignProperty = m_alignVToProperty.value(property)) {
        const Qt::Alignment v = m_alignValues.value(alignProperty);
        const Qt::Alignment newValue = indexVToAlign(value.toInt()) | indexHToAlign(alignToIndexH(v));
        if (v == newValue)
            return;

        variantProperty(alignProperty)->setValue(QVariant::fromValue(newValue));
    }
}

void VariantPropertyManager::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *alignProperty = m_alignHToProperty.value(property)) {
        m_propertyToAlignH.remove(alignProperty);
        m_alignHToProperty.remove(property);
    } else if (QtProperty *alignProperty = m_alignVToProperty.value(property)) {
        m_propertyToAlignV.remove(alignProperty);
        m_alignVToProperty.remove(property);
    }
}

int VariantPropertyManager::alignToIndexH(Qt::Alignment align) const
{
    if (align & Qt::AlignLeft)
        return 0;
    if (align & Qt::AlignHCenter)
        return 1;
    if (align & Qt::AlignRight)
        return 2;
    if (align & Qt::AlignJustify)
        return 3;
    return 0;
}

int VariantPropertyManager::alignToIndexV(Qt::Alignment align) const
{
    if (align & Qt::AlignTop)
        return 0;
    if (align & Qt::AlignVCenter)
        return 1;
    if (align & Qt::AlignBottom)
        return 2;
    return 1;
}

Qt::Alignment VariantPropertyManager::indexHToAlign(int idx) const
{
    switch (idx) {
    case 0: return Qt::AlignLeft;
    case 1: return Qt::AlignHCenter;
    case 2: return Qt::AlignRight;
    case 3: return Qt::AlignJustify;
    default: break;
    }
    return Qt::AlignLeft;
}

Qt::Alignment VariantPropertyManager::indexVToAlign(int idx) const
{
    switch (idx) {
    case 0: return Qt::AlignTop;
    case 1: return Qt::AlignVCenter;
    case 2: return Qt::AlignBottom;
    default: break;
    }
    return Qt::AlignVCenter;
}

QString VariantPropertyManager::indexHToString(int idx) const
{
    switch (idx) {
    case 0: return tr("Left");
    case 1: return tr("Center");
    case 2: return tr("Right");
    case 3: return tr("Justify");
    default: break;
    }
    return tr("Left");
}

QString VariantPropertyManager::indexVToString(int idx) const
{
    switch (idx) {
    case 0: return tr("Top");
    case 1: return tr("Center");
    case 2: return tr("Bottom");
    default: break;
    }
    return tr("Center");
}

} // namespace Tiled

#include "moc_variantpropertymanager.cpp"
