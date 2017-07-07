/*
 * wangtemplateview.h
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

#include "wangtemplatemodel.h"

#include <QTableView>

namespace Tiled {

class WangSet;
class WangId;

namespace Internal {

class WangTemplateView : public QTableView
{
    Q_OBJECT

public:
    WangTemplateView(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model) override;
    void updateBackgroundColor();

    WangTemplateModel *wangTemplateModel() const
    { return static_cast<WangTemplateModel *>(model()); }

    WangSet *wangSet() const;

    bool wangIdIsUsed(WangId wangId) const;

public slots:
    void wangIdUsed(WangId wangId);
    void wangIdRemoved(WangId wangId);

private:
    QHash<unsigned, bool> mUsedWangIds;
};

}
}
