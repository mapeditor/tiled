/*
 * Orx Engine Tiled Plugin
 * Copyright 2017, Denis Brachet aka Ainvar <thegwydd@gmail.com>
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

#ifndef ORX_NAME_GENERATOR_H
#define ORX_NAME_GENERATOR_H

#include <QString>

namespace Orx
{
struct NameGenerator
{
    // lazy name generator...
    static QString Generate(const QString & expression, QString name, int index, int column, int row)
    {
        QString ret;

        for (int a=0; a<expression.length(); ++a)
        {
            QChar c = expression[a];
            if (c == '%')
            {
                if ((a + 1) < expression.length())
                {
                    c = expression[++a];
                    if (c == 'n')
                        ret += name;
                    else if (c == 'i')
                        ret += QString::number(index);
                    else if (c == 'c')
                        ret += QString::number(column);
                    else if (c == 'r')
                        ret += QString::number(row);
                    else
                        ret += '%' + c;
                }
            }
            else
                ret += c;
        }

        return ret;
    }

};

} // namespace Orx

#endif // ORX_NAME_GENERATOR_H
