/*
 * converterdatamodel.h
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

#ifndef CONVERTERDATAMODEL_H
#define CONVERTERDATAMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

class ConverterControl;

class ConverterDataModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ConverterDataModel(ConverterControl *control, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    void insertFileNames(const QStringList &fileNames);

    int count() const { return mFileNames.count(); }

    QString fileName(int i) const { return mFileNames.at(i); }

    QString versionOfFile(const QString &fileName) const
    { return mFileVersions[fileName]; }

public slots:
    void updateVersions();

private:
    ConverterControl *mControl;
    QList<QString> mFileNames;
    QMap<QString, QString> mFileVersions;
};

#endif // CONVERTERDATAMODEL_H
