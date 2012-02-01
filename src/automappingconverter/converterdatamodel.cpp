/*
 * converterdatamodel.cpp
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

#include "converterdatamodel.h"
#include "convertercontrol.h"

#include <QDebug>

ConverterDataModel::ConverterDataModel(ConverterControl *control, QObject *parent)
    : QAbstractListModel(parent)
{
    mControl = control;
}

int ConverterDataModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mFileNames.count();
}

int ConverterDataModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant ConverterDataModel::data(const QModelIndex &index, int role) const
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
            return fileName;
        else if (columnIndex == 1)
            return mFileVersions[fileName];
        else
            return QVariant();
    }
    default:
        return QVariant();
    }
}

QVariant ConverterDataModel::headerData(int section, Qt::Orientation orientation,
                                        int role) const
{
    if (role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("File");
            break;
        case 1:
            return tr("Version");
            break;
        }
    }
    return QAbstractListModel::headerData(section, orientation, role);
}

void ConverterDataModel::insertFileNames(const QStringList &fileNames)
{
    const int row = mFileNames.size();
    beginInsertRows(QModelIndex(), row, row + fileNames.count() - 1);
    mFileNames.append(fileNames);
    foreach (const QString &fileName, fileNames)
         mFileVersions[fileName] = mControl->automappingRuleFileVersion(fileName);
    endInsertRows();
}

void ConverterDataModel::updateVersions()
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
