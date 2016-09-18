/*
 * rtbvalidatormodel.cpp
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#include "rtbvalidatormodel.h"

#include "mapdocument.h"

using namespace Tiled;
using namespace Tiled::Internal;

RTBValidatorModel::RTBValidatorModel(QObject *parent)
        : QAbstractListModel(parent)
        , mMapDocument(0)
        , mMaxMessageLenght(0)
{
}

QVariant RTBValidatorModel::data(const QModelIndex &index, int role) const
{
    RTBValidatorRule *rule = mRules.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
    {
        if(rule->mapObject())
            return QLatin1String("ID ") + QString::number(rule->mapObject()->id())
                    + QLatin1String(": ") + rule->message();
        else
            return rule->message();
    }
    case Qt::DecorationRole:
        return rule->symbol();
    default:
        return QVariant();
    }
}

Qt::ItemFlags RTBValidatorModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QAbstractListModel::flags(index);
        rc |= Qt::ItemIsSelectable;
    return rc;
}

int RTBValidatorModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mRules.size();
}

QVariant RTBValidatorModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Validator");
        }
    }
    return QVariant();
}

void RTBValidatorModel::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    beginResetModel();
    mMapDocument = mapDocument;
    endResetModel();
}

void RTBValidatorModel::appendRule(RTBValidatorRule *rule)
{
    beginInsertRows(QModelIndex(), mRules.size(), mRules.size());
    mRules.append(rule);
    endInsertRows();

    // set max message size
    int lenght = rule->message().length();
    if(lenght > maxMessageLenght())
        setMaxMessageLenght(lenght);
}

void RTBValidatorModel::clearRules()
{
    beginRemoveRows(QModelIndex(), 0, mRules.size());
    mRules.clear();
    endRemoveRows();

    // clear max message size
    setMaxMessageLenght(0);
}

MapObject *RTBValidatorModel::findMapObject(int row)
{
    return mRules.at(row)->mapObject();
}

RTBValidatorRule *RTBValidatorModel::findRule(int row)
{
    return mRules.at(row);
}

void RTBValidatorModel::highlightRules()
{
    for(RTBValidatorRule *rule : mRules)
    {
        if(rule->ruleID() == RTBValidatorRule::StartLocation)
            emit highlightToolbarAction(RTBMapObject::StartLocation);
        else if(rule->ruleID() == RTBValidatorRule::FinishHole)
            emit highlightToolbarAction(RTBMapObject::FinishHole);
    }
}
