/*
 * wangtemplatemodel.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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

#include "wangset.h"

#include <QAbstractListModel>

namespace Tiled {

/**
 * A model for getting the info for a wang set template of a given WangSet
 */
class WangTemplateModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum UserRoles {
        WangIdRole = Qt::UserRole
    };

    WangTemplateModel(WangSet *wangSet, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    WangId wangIdAt(const QModelIndex &index) const;

    QModelIndex wangIdIndex(WangId wangId) const;

    WangSet *wangSet() const { return mWangSet; }
    void setWangSet(WangSet *wangSet);

public slots:
    void wangSetChanged();

private:
    WangSet *mWangSet;
};

} // namespace Tiled
