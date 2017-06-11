/*
 * objecttemplatemodel.cpp
 * Copyright 2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Mohamed Thabet <thabetx@gmail.com>
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

#include "objecttemplatemodel.h"

#include "templategroup.h"

ObjectTemplateModel::ObjectTemplateModel(const TemplateGroup *templateGroup, QObject *parent):
    QAbstractListModel(parent),
    mTemplateGroup(templateGroup)
{
}

int ObjectTemplateModel::rowCount(const QModelIndex &parent) const
{
    return mTemplateGroup->templateCount();
}

QVariant ObjectTemplateModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= mTemplateGroup->templateCount())
        return QVariant();

    if (role == Qt::DisplayRole)
        return mTemplateGroup->templates().at(index.row())->name();
    else
        return QVariant();
}

