/*
 * reversingproxymodel.h
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

#include <QSortFilterProxyModel>

namespace Tiled {

/**
 * Displays the source model "upside down".
 */
class ReversingProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ReversingProxyModel(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &sourceLeft,
                  const QModelIndex &sourceRight) const override;
};

} // namespace Tiled
