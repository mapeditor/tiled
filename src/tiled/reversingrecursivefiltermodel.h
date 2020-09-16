/*
 * reversingrecursivefiltermodel.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "reversingproxymodel.h"

namespace Tiled {

class ReversingRecursiveFilterModel : public ReversingProxyModel
{
    Q_OBJECT

public:
    ReversingRecursiveFilterModel(QObject *parent = nullptr)
        : ReversingProxyModel(parent)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        setRecursiveFilteringEnabled(true);
#endif
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        return filterRecursiveAcceptsRow(sourceRow, sourceParent);
    }

private:
    bool filterRecursiveAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        if (ReversingProxyModel::filterAcceptsRow(sourceRow, sourceParent))
            return true;

        const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        const int count = sourceModel()->rowCount(index);

        for (int i = 0; i < count; ++i)
            if (filterRecursiveAcceptsRow(i, index))
                return true;

        return false;
    }
#endif
};

} // namespace Tiled
