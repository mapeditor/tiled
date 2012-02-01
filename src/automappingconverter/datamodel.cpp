/*
 * datamodel.cpp
 * Copyright 2012, Stefan Beller, stefanbeller@googlemail.com
 *
 * This file is part of the AutomappingConverter, which converts old rulemaps
 * of Tiled to work with the latest version of Tiled.
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

#include "datamodel.h"
#include "control.h"
#include <QDebug>

DataModel::DataModel(Control *control)
{
    mControl = control;
}

int DataModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mFileNames.count();
}

int DataModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant DataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const int rowIndex = index.row();
    const int columnIndex = index.column();

    if (rowIndex < 0 || rowIndex > mFileNames.count())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole: {
        const QString fileName = mFileNames.at(rowIndex);
        if (columnIndex == 0)
            return mFileVersions[fileName];
        else if (columnIndex == 1)
            return fileName;
        else
            return QVariant();
    }
    default:
        return QVariant();
    }
}

void DataModel::insertFileNames(const QList<QString> fileNames)
{
    const int row = mFileNames.size();
    beginInsertRows(QModelIndex(), row, row + fileNames.count() - 1);
    mFileNames.append(fileNames);
    foreach (const QString &fileName, fileNames)
         mFileVersions[fileName] = mControl->automappingRuleFileVersion(fileName);
    endInsertRows();
}

void DataModel::updateVersions()
{
    for (int i = 0; i < count(); ++i) {
        const QString fileName = mFileNames.at(i);
        const QString version = mFileVersions[fileName];
        qWarning() << "processing" << fileName << "at version" << version;
        if (version == mControl->version1()) {
            mControl->convertV1toV2(fileName);
            mFileVersions[fileName] = mControl->version2();
        }
    }
    emit dataChanged(index(0), index(count()));
}
