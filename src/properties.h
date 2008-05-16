/*
 * Tiled Map Editor (Qt port)
 * Copyright 2008 Tiled (Qt port) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt port).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QMap>
#include <QString>

namespace Tiled {

/**
 * A class holding a set of properties.
 */
class Properties
{
    public:
        /**
         * Destructor.
         */
        virtual ~Properties() {}

        /**
         * Get the value of a property.
         *
         * @param name The name of the property.
         * @param def  Default value, empty string by default.
         * @return the value of the given property or the given default when it
         *         doesn't exist.
         */
        const QString& getProperty(const QString &name,
                                   const QString &def = "") const
        {
            QMap<QString, QString>::const_iterator i = mProperties.find(name);
            return (i != mProperties.end()) ? i.value() : def;
        }

        /**
         * Returns whether a certain property exists.
         *
         * @param name The name of the property.
         * @return <code>true</code> when a property is defined,
         *         <code>false</code> otherwise.
         */
        bool hasProperty(const QString &name) const
        {
            return mProperties.contains(name);
        }

        /**
         * Set the value of a property.
         *
         * @param name  The name of the property.
         * @param value The value of the property.
         */
        void setProperty(const QString &name, const QString &value)
        {
            mProperties.insert(name, value);
        }

    private:
        QMap<QString, QString> mProperties;
};

} // namespace Tiled

#endif // PROPERTIES_H
