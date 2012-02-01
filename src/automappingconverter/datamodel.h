/*
 * datamodel.h
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

#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QMap>
#include <QString>

#include "control.h"

class DataModel : public QAbstractListModel
{
    Q_OBJECT

public:

    DataModel(Control *control);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const;

    void insertFileNames(const QList<QString> fileNames);

    int count() const { return mFileNames.count(); }

    QString fileName(int i) const { return mFileNames.at(i); }

    QString versionOfFile(const QString &fileName) const
    { return mFileVersions[fileName]; }

public slots:
    void updateVersions();

private:
    Control *mControl;
    QList<QString> mFileNames;
    QMap<QString, QString> mFileVersions;
};

#endif // DATAMODEL_H
