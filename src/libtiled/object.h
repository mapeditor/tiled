/*
 * object.h
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef OBJECT_H
#define OBJECT_H

#include "properties.h"

namespace Tiled {

/**
 * The base class for anything that can hold properties.
 */
class TILEDSHARED_EXPORT Object
{
public:
    /**
     * Virtual destructor.
     */
    virtual ~Object() {}

    /**
     * Returns the properties of this object.
     */
    const Properties &properties() const { return mProperties; }

    /**
     * Replaces all existing properties with a new set of properties.
     */
    void setProperties(const Properties &properties)
    { mProperties = properties; }

    /**
     * Merges \a properties with the existing properties. Properties with the
     * same name will be overridden.
     *
     * \sa Properties::merge
     */
    void mergeProperties(const Properties &properties)
    { mProperties.merge(properties); }

    /**
     * Returns the value of the object's \a name property.
     */
    QString property(const QString &name) const
    { return mProperties.value(name); }

    /**
     * Sets the value of the object's \a name property to \a value.
     */
    void setProperty(const QString &name, const QString &value)
    { mProperties.insert(name, value); }

private:
    Properties mProperties;
};

} // namespace Tiled

#endif // OBJECT_H
